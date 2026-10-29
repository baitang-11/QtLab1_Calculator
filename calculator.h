#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>

/**
 * @brief 计算器核心逻辑（与界面无关，可脱离 GUI 单独测试）。
 *
 * 状态说明：
 *  - m_entry       正在输入的数字文本（保留用户输入的写法，例如 "0.10"）
 *  - m_accumulator 第一个操作数，或连续运算时的中间结果
 *  - m_pendingOp   已按下但尚未执行的运算符（"+" "−" "×" "÷"），空表示没有
 *  - m_freshEntry  true 表示下一次输入数字应当重新开始写，而不是追加
 *  - m_justEvaluated 刚按过等号，用于"计算完成后继续输入"的判断
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

    QString displayText() const;             // 主显示区文本（输入的数据或计算结果）
    QString expressionText() const;          // 上方表达式文本（操作数 + 运算符）

private:
    double currentValue() const;             // 当前输入框数值
    void applyPendingOperation();            // 执行 m_pendingOp，结果写回 m_entry

    static QString formatNumber(double value);

    QString m_entry;
    double m_accumulator;
    QString m_pendingOp;
    QString m_expression;
    bool m_freshEntry;
    bool m_justEvaluated;
};

#endif // CALCULATOR_H
