// 自动化测试：CalculatorEngine 逻辑单元测试 + MainWindow 鼠标/键盘集成测试。
//
// 直接复用工程源码（见 tests.pro），因此测试通过即说明真实程序行为正确。
#include <QtTest>
#include <QApplication>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "calculator.h"
#include "mainwindow.h"

namespace {

// 用紧凑脚本驱动引擎：数字 0-9、"." "+" "-" "*" "/" "=" 退格 "<" 清除 "c" 正负号 "n"
void driveScript(CalculatorEngine &engine, const QString &script)
{
    static const QHash<QChar, QString> operators = {
        { QLatin1Char('+'), QStringLiteral("+") },
        { QLatin1Char('-'), QStringLiteral("−") },
        { QLatin1Char('*'), QStringLiteral("×") },
        { QLatin1Char('/'), QStringLiteral("÷") },
    };

    for (const QChar ch : script) {
        if (ch.isDigit()) {
            engine.inputDigit(ch.digitValue());
        } else if (ch == QLatin1Char('.')) {
            engine.inputDot();
        } else if (operators.contains(ch)) {
            engine.inputOperator(operators.value(ch));
        } else if (ch == QLatin1Char('=')) {
            engine.inputEquals();
        } else if (ch == QLatin1Char('<')) {
            engine.backspace();
        } else if (ch == QLatin1Char('c')) {
            engine.clearAll();
        } else if (ch == QLatin1Char('n')) {
            engine.toggleSign();
        }
    }
}

// 依次按下主窗口上的按钮（按对象名查找，验证界面与逻辑的连线是否正确）
void clickButtons(MainWindow &window, const QStringList &objectNames)
{
    for (const QString &name : objectNames) {
        QPushButton *button = window.findChild<QPushButton *>(name);
        if (!button) {
            QFAIL(qPrintable(QStringLiteral("找不到按钮 %1").arg(name)));
            return;
        }
        QTest::mouseClick(button, Qt::LeftButton);
    }
}

QString displayText(MainWindow &window)
{
    QLineEdit *display = window.findChild<QLineEdit *>(QStringLiteral("resultDisplay"));
    return display ? display->text() : QString();
}

QString expressionText(MainWindow &window)
{
    QLabel *label = window.findChild<QLabel *>(QStringLiteral("expressionLabel"));
    return label ? label->text() : QString();
}

} // namespace

/**
 * 引擎逻辑测试：覆盖四则运算、小数、退格、清除、连按运算符、除零、
 * 结果后继续输入等基础与边界情况。
 */
class EngineTest : public QObject
{
    Q_OBJECT

private slots:
    void initialState()
    {
        CalculatorEngine engine;
        QCOMPARE(engine.displayText(), QStringLiteral("0"));
        QCOMPARE(engine.expressionText(), QString());
        QVERIFY(!engine.hasError());
    }

    void inputDigitsAndDot()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("12.34"));
        QCOMPARE(engine.displayText(), QStringLiteral("12.34"));
    }

    void leadingZeroIsReplaced()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("007"));
        QCOMPARE(engine.displayText(), QStringLiteral("7"));
    }

    void duplicateDotIsIgnored()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("1.2."));

        // 提示是一次性的：必须在重复输入小数点的当下检查，随后续输入自动清除
        QVERIFY(engine.statusMessage().contains(QStringLiteral("小数点")));

        driveScript(engine, QStringLiteral("3"));
        QCOMPARE(engine.displayText(), QStringLiteral("1.23"));
        QVERIFY(engine.statusMessage().isEmpty());
    }

    void dotAfterOperatorStartsFraction()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("5+."));
        QCOMPARE(engine.displayText(), QStringLiteral("0."));
    }

    void fourOperations_data()
    {
        QTest::addColumn<QString>("script");
        QTest::addColumn<QString>("expected");

        QTest::newRow("add")      << QStringLiteral("12.5+7.5=")  << QStringLiteral("20");
        QTest::newRow("sub")      << QStringLiteral("5-8=")       << QStringLiteral("-3");
        QTest::newRow("mul")      << QStringLiteral("1.5*2.25=")  << QStringLiteral("3.375");
        QTest::newRow("div")      << QStringLiteral("1/4=")       << QStringLiteral("0.25");
        QTest::newRow("recur")    << QStringLiteral("1/3=")       << QStringLiteral("0.333333333333333");
    }

    void fourOperations()
    {
        QFETCH(QString, script);
        QFETCH(QString, expected);

        CalculatorEngine engine;
        driveScript(engine, script);
        QCOMPARE(engine.displayText(), expected);
    }

    void floatingPointNoiseIsHidden()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("0.1+0.2="));
        QCOMPARE(engine.displayText(), QStringLiteral("0.3"));
    }

    void consecutiveOperatorsReplacePrevious()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("5+"));
        driveScript(engine, QStringLiteral("+"));

        // 同样在连按运算符的当下检查提示
        QVERIFY(engine.statusMessage().contains(QStringLiteral("替换")));

        driveScript(engine, QStringLiteral("8="));
        QCOMPARE(engine.displayText(), QStringLiteral("13"));
    }

    void divisionByZeroEntersErrorState()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("1/0="));
        QVERIFY(engine.hasError());
        QCOMPARE(engine.displayText(), QStringLiteral("无法除以零"));
        QCOMPARE(engine.expressionText(), QStringLiteral("1 ÷ 0 ="));
    }

    void digitAfterErrorStartsNewCalculation()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("8/0=7"));
        QVERIFY(!engine.hasError());
        QCOMPARE(engine.displayText(), QStringLiteral("7"));
    }

    void backspaceClearsErrorState()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("8/0<"));
        QVERIFY(!engine.hasError());
        QCOMPARE(engine.displayText(), QStringLiteral("0"));
    }

    void digitLimitIsEnforced()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("99999999999999999"));   // 17 个 9
        QCOMPARE(engine.displayText(), QStringLiteral("999999999999999"));   // 只保留 15 位
        QVERIFY(engine.statusMessage().contains(QStringLiteral("15")));
    }

    void backspaceWhileTyping()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("123<"));
        QCOMPARE(engine.displayText(), QStringLiteral("12"));

        driveScript(engine, QStringLiteral("<<"));
        QCOMPARE(engine.displayText(), QStringLiteral("0"));
    }

    void backspaceAfterResultDoesNotBreakResult()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("2*3=<"));
        QCOMPARE(engine.displayText(), QStringLiteral("6"));
        QVERIFY(!engine.statusMessage().isEmpty());
    }

    void continueCalculationAfterEquals()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("2*3=5+2="));
        QCOMPARE(engine.displayText(), QStringLiteral("7"));
    }

    void chainedOperationsUseIntermediateResult()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("7+2*3+="));
        QCOMPARE(engine.displayText(), QStringLiteral("54"));
    }

    void toggleSign()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("5n"));
        QCOMPARE(engine.displayText(), QStringLiteral("-5"));

        driveScript(engine, QStringLiteral("n"));
        QCOMPARE(engine.displayText(), QStringLiteral("5"));
    }

    void clearResetsEverything()
    {
        CalculatorEngine engine;
        driveScript(engine, QStringLiteral("12+34=5"));
        QCOMPARE(engine.displayText(), QStringLiteral("5"));

        driveScript(engine, QStringLiteral("c"));
        QCOMPARE(engine.displayText(), QStringLiteral("0"));
        QCOMPARE(engine.expressionText(), QString());
    }
};

/**
 * 界面集成测试：验证按钮连线、鼠标点击与键盘输入共用同一套逻辑。
 */
class WindowTest : public QObject
{
    Q_OBJECT

private slots:
    void everyDigitButtonMapsToItsDigit()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        for (int digit = 0; digit <= 9; ++digit) {
            clickButtons(window, { QStringLiteral("btnClear") });
            clickButtons(window, { QStringLiteral("btn%1").arg(digit) });
            QCOMPARE(displayText(window), QString::number(digit));
        }
    }

    void mouseClicksCalculateWithDecimals()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        clickButtons(window, { QStringLiteral("btnClear"),
                               QStringLiteral("btn1"), QStringLiteral("btnDot"),
                               QStringLiteral("btn5"), QStringLiteral("btnMultiply"),
                               QStringLiteral("btn2"), QStringLiteral("btnDot"),
                               QStringLiteral("btn2"), QStringLiteral("btn5"),
                               QStringLiteral("btnEquals") });

        QCOMPARE(displayText(window), QStringLiteral("3.375"));
        QCOMPARE(expressionText(window), QStringLiteral("1.5 × 2.25 ="));
    }

    void mouseBackspaceAndClear()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        clickButtons(window, { QStringLiteral("btnClear"),
                               QStringLiteral("btn1"), QStringLiteral("btn2"),
                               QStringLiteral("btn3"), QStringLiteral("btnBackspace") });
        QCOMPARE(displayText(window), QStringLiteral("12"));

        clickButtons(window, { QStringLiteral("btnClear") });
        QCOMPARE(displayText(window), QStringLiteral("0"));
    }

    void keyboardAndMouseProduceSameResult()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        // 鼠标输入
        clickButtons(window, { QStringLiteral("btnClear"),
                               QStringLiteral("btn1"), QStringLiteral("btnDot"),
                               QStringLiteral("btn5"), QStringLiteral("btnAdd"),
                               QStringLiteral("btn2"), QStringLiteral("btnDot"),
                               QStringLiteral("btn2"), QStringLiteral("btn5"),
                               QStringLiteral("btnEquals") });
        const QString mouseDisplay = displayText(window);
        const QString mouseExpression = expressionText(window);

        // 键盘输入同样的表达式
        clickButtons(window, { QStringLiteral("btnClear") });
        QTest::keyClicks(&window, QStringLiteral("1.5+2.25="));
        const QString keyboardDisplay = displayText(window);
        const QString keyboardExpression = expressionText(window);

        QCOMPARE(mouseDisplay, QStringLiteral("3.75"));
        QCOMPARE(keyboardDisplay, mouseDisplay);
        QCOMPARE(keyboardExpression, mouseExpression);
    }

    void keyboardSpecialKeys()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        // 输入 12 后退格，再输入 34，最后用回车计算
        QTest::keyClicks(&window, QStringLiteral("12"));
        QTest::keyClick(&window, Qt::Key_Backspace);
        QCOMPARE(displayText(window), QStringLiteral("1"));

        QTest::keyClick(&window, Qt::Key_Escape);
        QCOMPARE(displayText(window), QStringLiteral("0"));

        QTest::keyClicks(&window, QStringLiteral("7"));
        QTest::keyClick(&window, Qt::Key_Asterisk);
        QTest::keyClicks(&window, QStringLiteral("6"));
        QTest::keyClick(&window, Qt::Key_Return);
        QCOMPARE(displayText(window), QStringLiteral("42"));

        // F9 切换正负号
        QTest::keyClick(&window, Qt::Key_F9);
        QCOMPARE(displayText(window), QStringLiteral("-42"));
    }

    void keyboardDivisionByZeroShowsError()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QTest::keyClicks(&window, QStringLiteral("8/0="));
        QCOMPARE(displayText(window), QStringLiteral("无法除以零"));

        // 出错的显示区应带上样式属性，样式表据此显示红色
        QLineEdit *display = window.findChild<QLineEdit *>(QStringLiteral("resultDisplay"));
        QVERIFY(display);
        QVERIFY(display->property("calcError").toBool());
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    EngineTest engineTest;
    WindowTest windowTest;

    int failed = 0;
    failed += QTest::qExec(&engineTest, argc, argv);
    failed += QTest::qExec(&windowTest, argc, argv);
    return failed;
}

#include "tst_calculator.moc"
