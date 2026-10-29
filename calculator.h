#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>

/**
 * @brief 计算器核心逻辑（与界面无关，可脱离 GUI 单独测试）。
 *
 * 状态说明：
 *  - m_entry         正在输入的数字文本（保留用户输入的写法，例如 "0.10"）
 *  - m_accumulator   第一个操作数，或连续运算时的中间结果
 *  - m_pendingOp     已按下但尚未执行的运算符（"+" "−" "×" "÷"），空表示没有
 *  - m_freshEntry    true 表示下一次输入数字应当重新开始写，而不是追加
 *  - m_justEvaluated 刚按过等号，用于"计算完成后继续输入"的判断
 *  - m_errorState    除零、结果溢出等错误状态，此时只接受清除或重新输入
 *  - m_statusMessage 一次性的提示信息，例如忽略重复小数点、替换连续运算符
 */
class CalculatorEngine
{
public:
    CalculatorEngine();

    void inputDigit(int digit);              // 输入数字 0-9
    void inputDot();                         // 输入小数点
    void inputOperator(const QString &op);   // 输入运算符
    void inputEquals();                      // 按等号
    void backspace();                        // 退格
    void clearAll();                         // 清除（C）
    void toggleSign();                       // 正负号切换

    QString displayText() const;             // 主显示区文本（输入的数据、结果或错误信息）
    QString expressionText() const;          // 上方表达式文本（操作数 + 运算符）
    QString statusMessage() const;           // 异常输入的一次性提示，无提示时为空串
    bool hasError() const;                   // 当前是否处于错误状态

private:
    double currentValue() const;             // 当前输入框数值
    bool applyPendingOperation();            // 执行 m_pendingOp，失败返回 false
    void startFreshCalculation();            // 结果后继续输入时，丢弃上一次计算的状态
    void setError(const QString &message);   // 进入错误状态

    static QString formatNumber(double value);
    static int digitCount(const QString &text);

    QString m_entry;
    double m_accumulator;
    QString m_pendingOp;
    QString m_expression;
    QString m_statusMessage;
    bool m_freshEntry;
    bool m_justEvaluated;
    bool m_errorState;
};

#endif // CALCULATOR_H
