#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "calculator.h"

class QKeyEvent;

namespace Ui {
class MainWindow;
}

/**
 * @brief 计算器主窗口。
 *
 * 界面由 Qt Designer 文件 mainwindow.ui 描述（显示区 + 4×5 按键网格）。
 * 鼠标点击与键盘输入都先翻译成统一的输入记号，再交给 dispatchInput() 处理，
 * 因此两条输入路径复用同一套逻辑，不会出现"鼠标能用、键盘不能用"的差异。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // 键盘事件统一入口：把按键翻译成输入记号后转交 dispatchInput()
    void keyPressEvent(QKeyEvent *event) override;

private:
    // 统一的输入分发入口：鼠标点击与键盘输入都调用它
    void dispatchInput(const QString &token);
    void refreshDisplay();

    // 把一次按键翻译成与按钮一致的输入记号，无法识别时返回空串
    static QString tokenFromKeyEvent(const QKeyEvent *event);

    Ui::MainWindow *ui;
    CalculatorEngine m_engine;
};

#endif // MAINWINDOW_H
