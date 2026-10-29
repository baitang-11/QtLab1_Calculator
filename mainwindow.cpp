#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHash>
#include <QKeyEvent>
#include <QPushButton>
#include <QStyle>

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

// 没有异常提示时，窗口底部显示的操作说明
const QString &defaultHintText()
{
    static const QString text =
        QStringLiteral("键盘：数字 . + − * ÷，Enter 计算，Backspace 退格，Esc 清除");
    return text;
}

// 样式表中的动态属性改变后，必须重新计算样式才能立即生效
void repolish(QWidget *widget)
{
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
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

    // 正常时显示键盘操作提示，出现异常输入时临时显示提示信息
    const QString message = m_engine.statusMessage();
    ui->hintLabel->setText(message.isEmpty() ? defaultHintText() : message);

    // 通过动态属性驱动样式表：出错时主显示区变红，异常提示时提示文字变红
    const bool hasError = m_engine.hasError();
    const bool hasNotice = !message.isEmpty();
    if (ui->resultDisplay->property("calcError").toBool() != hasError) {
        ui->resultDisplay->setProperty("calcError", hasError);
        repolish(ui->resultDisplay);
    }
    if (ui->hintLabel->property("notice").toBool() != hasNotice) {
        ui->hintLabel->setProperty("notice", hasNotice);
        repolish(ui->hintLabel);
    }
}

/**
 * 把一次按键翻译成输入记号。
 * 优先看 event->text()：这样主键盘和小键盘、Shift+= 得到 '+' 等情况都能自然统一；
 * 对没有可见字符的功能键（退格、回车、Esc）再回退到 event->key() 判断。
 */
QString MainWindow::tokenFromKeyEvent(const QKeyEvent *event)
{
    const QString text = event->text();
    if (text.size() == 1) {
        const QChar ch = text.at(0);
        if (ch.isDigit()) {
            return QString(ch);
        }
        switch (ch.unicode()) {
        case '.':                       // 主键盘句号
        case ',':                       // 部分布局的逗号同样当小数点
            return QStringLiteral(".");
        case '+':
            return QStringLiteral("+");
        case '-':
            return QStringLiteral("−");  // 统一成界面上的减号符号
        case '*':
        case 'x':
        case 'X':
            return QStringLiteral("×");
        case '/':
            return QStringLiteral("÷");
        case '=':
            return QStringLiteral("=");
        case 'c':
        case 'C':
            return QStringLiteral("CLEAR");
        default:
            break;
        }
    }

    switch (event->key()) {
    case Qt::Key_Backspace:
        return QStringLiteral("BACK");
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Equal:
        return QStringLiteral("=");
    case Qt::Key_Escape:
    case Qt::Key_Delete:
        return QStringLiteral("CLEAR");
    case Qt::Key_F9:
        return QStringLiteral("SIGN");
    default:
        break;
    }
    return QString();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const QString token = tokenFromKeyEvent(event);
    if (token.isEmpty()) {
        // 不是计算器关心的按键，交回基类处理（例如 Ctrl+Q 等快捷键）
        QMainWindow::keyPressEvent(event);
        return;
    }

    event->accept();
    dispatchInput(token);   // 与鼠标点击走同一个入口，保证两种输入行为完全一致
}
