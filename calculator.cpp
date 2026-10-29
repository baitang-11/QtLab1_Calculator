#include "calculator.h"

namespace {
// 运算符常量：使用参考文档中的 Unicode 符号，避免界面上出现 * 和 /
const QString kAdd = QStringLiteral("+");
const QString kSub = QStringLiteral("−");   // U+2212 MINUS SIGN
const QString kMul = QStringLiteral("×");   // U+00D7 MULTIPLICATION SIGN
const QString kDiv = QStringLiteral("÷");   // U+00F7 DIVISION SIGN
}

CalculatorEngine::CalculatorEngine()
    : m_entry(QStringLiteral("0"))
    , m_accumulator(0.0)
    , m_pendingOp()
    , m_expression()
    , m_freshEntry(false)
    , m_justEvaluated(false)
{
}

double CalculatorEngine::currentValue() const
{
    return m_entry.toDouble();
}

QString CalculatorEngine::formatNumber(double value)
{
    return QString::number(value, 'g', 15);
}

void CalculatorEngine::inputDigit(int digit)
{
    // 计算完成后继续输入：当作一次新的计算
    if (m_justEvaluated) {
        m_entry.clear();
        m_accumulator = 0.0;
        m_pendingOp.clear();
        m_expression.clear();
        m_justEvaluated = false;
        m_freshEntry = true;
    }

    if (m_freshEntry) {
        m_entry = QString::number(digit);
        m_freshEntry = false;
    } else if (m_entry == QStringLiteral("0")) {
        m_entry = QString::number(digit);
    } else {
        m_entry += QString::number(digit);
    }
}

void CalculatorEngine::inputDot()
{
    if (m_justEvaluated) {
        m_entry.clear();
        m_accumulator = 0.0;
        m_pendingOp.clear();
        m_expression.clear();
        m_justEvaluated = false;
        m_freshEntry = true;
    }

    if (m_freshEntry) {
        m_entry = QStringLiteral("0.");
        m_freshEntry = false;
    } else {
        m_entry += QLatin1Char('.');
    }
}

void CalculatorEngine::inputOperator(const QString &op)
{
    if (m_justEvaluated) {
        // 连续运算：上一次的结果作为新的第一个操作数
        m_accumulator = currentValue();
        m_justEvaluated = false;
    } else if (!m_pendingOp.isEmpty() && !m_freshEntry) {
        applyPendingOperation();
    } else {
        m_accumulator = currentValue();
    }

    m_pendingOp = op;
    m_freshEntry = true;
    m_expression = formatNumber(m_accumulator) + QLatin1Char(' ') + op;
}

void CalculatorEngine::applyPendingOperation()
{
    const double rhs = currentValue();
    double result = m_accumulator;

    if (m_pendingOp == kAdd) {
        result = m_accumulator + rhs;
    } else if (m_pendingOp == kSub) {
        result = m_accumulator - rhs;
    } else if (m_pendingOp == kMul) {
        result = m_accumulator * rhs;
    } else if (m_pendingOp == kDiv) {
        result = m_accumulator / rhs;
    }

    m_expression = formatNumber(m_accumulator) + QLatin1Char(' ') + m_pendingOp
                   + QLatin1Char(' ') + formatNumber(rhs) + QStringLiteral(" =");
    m_accumulator = result;
    m_entry = formatNumber(result);
    m_pendingOp.clear();
    m_freshEntry = true;
}

void CalculatorEngine::inputEquals()
{
    if (!m_pendingOp.isEmpty()) {
        applyPendingOperation();
    }
    m_justEvaluated = true;
}

void CalculatorEngine::backspace()
{
    m_entry.chop(1);
    if (m_entry.isEmpty() || m_entry == QStringLiteral("-")) {
        m_entry = QStringLiteral("0");
    }
}

void CalculatorEngine::clearAll()
{
    m_entry = QStringLiteral("0");
    m_accumulator = 0.0;
    m_pendingOp.clear();
    m_expression.clear();
    m_freshEntry = false;
    m_justEvaluated = false;
}

void CalculatorEngine::toggleSign()
{
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
