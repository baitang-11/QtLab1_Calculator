#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "calculator.h"

namespace Ui {
class MainWindow;
}

/**
 * @brief 计算器主窗口。
 *
 * 界面由 Qt Designer 文件 mainwindow.ui 描述（显示区 + 4×5 按键网格），
 * 本类负责把界面事件统一分发到 CalculatorEngine，并刷新显示。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    // 统一的输入分发入口：鼠标点击与键盘输入都调用它
    void dispatchInput(const QString &token);
    void refreshDisplay();

    Ui::MainWindow *ui;
    CalculatorEngine m_engine;
};

#endif // MAINWINDOW_H
