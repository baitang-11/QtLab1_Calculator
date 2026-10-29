#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 让窗口本身持有键盘焦点：所有按钮与显示框都设置为 NoFocus，
    // 这样无论用鼠标点了哪个按钮，键盘事件都会送到 keyPressEvent()。
    setFocusPolicy(Qt::StrongFocus);
    setWindowTitle(QStringLiteral("计算器"));
    ui->resultDisplay->setText(QStringLiteral("0"));
    setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}
