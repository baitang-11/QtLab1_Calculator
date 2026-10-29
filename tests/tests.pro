QT       += core gui widgets testlib

CONFIG   += c++11 console
CONFIG   -= app_bundle

TEMPLATE  = app
TARGET    = tst_calculator

# 复用工程源码：测试直接针对真实的 CalculatorEngine 与 MainWindow
INCLUDEPATH += ..

SOURCES += \
    tst_calculator.cpp \
    ../calculator.cpp \
    ../mainwindow.cpp

HEADERS += \
    ../calculator.h \
    ../mainwindow.h

FORMS += \
    ../mainwindow.ui

RESOURCES += \
    ../resources.qrc
