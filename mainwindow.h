#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
