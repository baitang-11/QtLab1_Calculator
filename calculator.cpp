#include "calculator.h"

#include <cmath>

namespace {
// 运算符常量：使用参考文档中的 Unicode 符号，避免界面上出现 * 和 /
const QString kAdd = QStringLiteral("+");
const QString kSub = QStringLiteral("−");   // U+2212 MINUS SIGN
const QString kMul = QStringLiteral("×");   // U+00D7 MULTIPLICATION SIGN
const QString kDiv = QStringLiteral("÷");   // U+00F7 DIVISION SIGN

// 单个操作数最多 15 位有效数字，与 double 的十进制精度相匹配
const int kMaxDigits = 15;

const QString kZero = QStringLiteral("0");
const QString kNegativeZero = QStringLiteral("-0");
} // namespace

CalculatorEngine::CalculatorEngine()
    : m_entry(kZero)
    , m_accumulator(0.0)
    , m_pendingOp()
    , m_expression()
    , m_statusMessage()
    , m_freshEntry(false)
    , m_justEvaluated(false)
    , m_errorState(false)
{
}

double CalculatorEngine::currentValue() const
{
    return m_entry.toDouble();
}

int CalculatorEngine::digitCount(const QString &text)
{
    int count = 0;
    for (const QChar ch : text) {
        if (ch.isDigit()) {
            ++count;
        }
    }
    return count;
}

/**
 * 结果格式化：整数按整数输出，其余用 15 位有效数字并去掉尾部多余的 0。
 * 这样 0.1 + 0.2 显示为 0.3，而不是 0.30000000000000004。
 */
QString CalculatorEngine::formatNumber(double value)
{
    if (!std::isfinite(value)) {
        return QString();
    }
    // -0.0 也一并归一化为 0
    if (std::fabs(value) < 1e15 && value == std::floor(value)) {
        return QString::number(static_cast<qlonglong>(value));
    }
    return QString::number(value, 'g', 15);
}

void CalculatorEngine::startFreshCalculation()
{
    m_entry = kZero;
    m_accumulator = 0.0;
    m_pendingOp.clear();
    m_expression.clear();
    m_freshEntry = true;
    m_justEvaluated = false;
}

void CalculatorEngine::setError(const QString &message)
{
    m_errorState = true;
    m_entry = message;
    m_pendingOp.clear();
    m_freshEntry = true;
    m_justEvaluated = false;
}

void CalculatorEngine::inputDigit(int digit)
{
    m_statusMessage.clear();

    if (m_errorState) {
        // 错误状态下按数字，直接开始一次新的计算
        clearAll();
    } else if (m_justEvaluated) {
        startFreshCalculation();
    }

    const QString text = QString::number(digit);

    if (m_freshEntry) {
        m_entry = text;
        m_freshEntry = false;
        return;
    }
    if (m_entry == kZero) {
        m_entry = text;
        return;
    }
    if (m_entry == kNegativeZero) {
        // 负号后面直接接数字，避免出现 -05
        m_entry = QStringLiteral("-") + text;
        return;
    }
    if (digitCount(m_entry) >= kMaxDigits) {
        m_statusMessage = QStringLiteral("最多只能输入 %1 位数字").arg(kMaxDigits);
        return;
    }
    m_entry += text;
}

void CalculatorEngine::inputDot()
{
    m_statusMessage.clear();

    if (m_errorState) {
        clearAll();
    } else if (m_justEvaluated) {
        startFreshCalculation();
    }

    if (m_freshEntry) {
        m_entry = QStringLiteral("0.");
        m_freshEntry = false;
        return;
    }
    if (m_entry.contains(QLatin1Char('.'))) {
        // 同一个操作数只允许一个小数点
        m_statusMessage = QStringLiteral("同一个数字不能重复输入小数点");
        return;
    }
    m_entry += QLatin1Char('.');
}

void CalculatorEngine::inputOperator(const QString &op)
{
    m_statusMessage.clear();

    if (m_errorState) {
        // 错误状态下不接受运算符，必须先清除或重新输入
        return;
    }

    if (m_justEvaluated) {
        // 连续运算：把上一次的结果当作第一个操作数
        m_accumulator = currentValue();
        m_justEvaluated = false;
    } else if (m_pendingOp.isEmpty()) {
        m_accumulator = currentValue();
    } else if (m_freshEntry) {
        // 连按运算符：用新的运算符覆盖上一个，而不是叠加执行
        m_statusMessage = QStringLiteral("连续输入运算符，已替换为 %1").arg(op);
    } else if (!applyPendingOperation()) {
        // 先结算上一个运算（允许 2 + 3 × 4 逐次求值），失败则直接返回
        return;
    }

    m_pendingOp = op;
    m_freshEntry = true;
    m_expression = formatNumber(m_accumulator) + QLatin1Char(' ') + op;
}

bool CalculatorEngine::applyPendingOperation()
{
    const double lhs = m_accumulator;
    const double rhs = currentValue();
    const QString op = m_pendingOp;
    double result = 0.0;

    if (op == kAdd) {
        result = lhs + rhs;
    } else if (op == kSub) {
        result = lhs - rhs;
    } else if (op == kMul) {
        result = lhs * rhs;
    } else if (op == kDiv) {
        if (rhs == 0.0) {
            m_expression = formatNumber(lhs) + QLatin1Char(' ') + op + QLatin1Char(' ')
                           + formatNumber(rhs) + QStringLiteral(" =");
            setError(QStringLiteral("无法除以零"));
            return false;
        }
        result = lhs / rhs;
    } else {
        return false;
    }

    m_expression = formatNumber(lhs) + QLatin1Char(' ') + op + QLatin1Char(' ')
                   + formatNumber(rhs) + QStringLiteral(" =");

    if (!std::isfinite(result)) {
        setError(QStringLiteral("结果超出范围"));
        return false;
    }

    m_accumulator = result;
    m_entry = formatNumber(result);
    m_pendingOp.clear();
    m_freshEntry = true;
    return true;
}

void CalculatorEngine::inputEquals()
{
    m_statusMessage.clear();

    if (m_errorState) {
        return;
    }
    if (m_pendingOp.isEmpty()) {
        if (m_justEvaluated) {
            m_statusMessage = QStringLiteral("计算结果已显示，请继续输入或按 C 清除");
        }
        return;
    }

    if (!applyPendingOperation()) {
        return;
    }
    m_justEvaluated = true;
}

void CalculatorEngine::backspace()
{
    m_statusMessage.clear();

    if (m_errorState) {
        // 错误状态下退格视为清除，避免用户被卡在错误状态里
        clearAll();
        return;
    }
    if (m_justEvaluated) {
        m_statusMessage = QStringLiteral("计算结果不能退格，可直接输入数字开始新的计算");
        return;
    }
    if (m_freshEntry) {
        // 刚按完运算符，还没有输入第二个操作数，无需退格
        return;
    }

    m_entry.chop(1);
    if (m_entry.isEmpty() || m_entry == QStringLiteral("-")) {
        m_entry = kZero;
    }
}

void CalculatorEngine::clearAll()
{
    m_entry = kZero;
    m_accumulator = 0.0;
    m_pendingOp.clear();
    m_expression.clear();
    m_statusMessage.clear();
    m_freshEntry = false;
    m_justEvaluated = false;
    m_errorState = false;
}

void CalculatorEngine::toggleSign()
{
    m_statusMessage.clear();

    if (m_errorState) {
        return;
    }
    if (m_entry.startsWith(QLatin1Char('-'))) {
        m_entry.remove(0, 1);
    } else {
        m_entry.prepend(QLatin1Char('-'));
    }
}

QString CalculatorEngine::displayText() const
{
    return m_entry;
}

QString CalculatorEngine::expressionText() const
{
    return m_expression;
}

QString CalculatorEngine::statusMessage() const
{
    return m_statusMessage;
}

bool CalculatorEngine::hasError() const
{
    return m_errorState;
}
