#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHash>
#include <QPushButton>

namespace {
/**
 * 按钮对象名 → 统一输入记号。
 * 界面上显示的是 "×" "÷" "−"，但分发时只认这一套记号，
 * 后续键盘输入也映射到同样的记号，保证两条输入路径复用同一份逻辑。
 */
const QHash<QString, QString> &buttonTokens()
{
    static const QHash<QString, QString> tokens = {
        { QStringLiteral("btn0"), QStringLiteral("0") },
        { QStringLiteral("btn1"), QStringLiteral("1") },
        { QStringLiteral("btn2"), QStringLiteral("2") },
        { QStringLiteral("btn3"), QStringLiteral("3") },
        { QStringLiteral("btn4"), QStringLiteral("4") },
        { QStringLiteral("btn5"), QStringLiteral("5") },
        { QStringLiteral("btn6"), QStringLiteral("6") },
        { QStringLiteral("btn7"), QStringLiteral("7") },
        { QStringLiteral("btn8"), QStringLiteral("8") },
        { QStringLiteral("btn9"), QStringLiteral("9") },
        { QStringLiteral("btnDot"), QStringLiteral(".") },
        { QStringLiteral("btnAdd"), QStringLiteral("+") },
        { QStringLiteral("btnSubtract"), QStringLiteral("−") },
        { QStringLiteral("btnMultiply"), QStringLiteral("×") },
        { QStringLiteral("btnDivide"), QStringLiteral("÷") },
        { QStringLiteral("btnEquals"), QStringLiteral("=") },
        { QStringLiteral("btnClear"), QStringLiteral("CLEAR") },
        { QStringLiteral("btnBackspace"), QStringLiteral("BACK") },
        { QStringLiteral("btnSign"), QStringLiteral("SIGN") },
    };
    return tokens;
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 让窗口本身持有键盘焦点：所有按钮与显示框都设置为 NoFocus，
    // 这样无论用鼠标点了哪个按钮，键盘事件都会送到 keyPressEvent()。
    setFocusPolicy(Qt::StrongFocus);
    setWindowTitle(QStringLiteral("计算器"));

    // 所有按钮连接到同一个槽（lambda 转发记号），避免为每个按钮写重复的处理代码
    const QHash<QString, QString> &tokens = buttonTokens();
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        const QString token = tokens.value(button->objectName());
        if (token.isEmpty()) {
            continue;
        }
        connect(button, &QPushButton::clicked, this, [this, token]() { dispatchInput(token); });
    }

    refreshDisplay();
    setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dispatchInput(const QString &token)
{
    if (token.size() == 1 && token.at(0).isDigit()) {
        m_engine.inputDigit(token.toInt());
    } else if (token == QStringLiteral(".")) {
        m_engine.inputDot();
    } else if (token == QStringLiteral("+") || token == QStringLiteral("−")
               || token == QStringLiteral("×") || token == QStringLiteral("÷")) {
        m_engine.inputOperator(token);
    } else if (token == QStringLiteral("=")) {
        m_engine.inputEquals();
    } else if (token == QStringLiteral("BACK")) {
        m_engine.backspace();
    } else if (token == QStringLiteral("CLEAR")) {
        m_engine.clearAll();
    } else if (token == QStringLiteral("SIGN")) {
        m_engine.toggleSign();
    }

    refreshDisplay();
}

void MainWindow::refreshDisplay()
{
    ui->resultDisplay->setText(m_engine.displayText());
    ui->expressionLabel->setText(m_engine.expressionText());
}
