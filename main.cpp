#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QtLab1Calculator"));
    app.setApplicationDisplayName(QStringLiteral("计算器"));

    MainWindow window;
    window.show();

    return app.exec();
}
