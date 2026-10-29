QT       += core gui

# Qt 5 需要显式引入 widgets 模块；Qt 6 中 widgets 已随 core gui 提供，
# 该判断使工程同时适用于 Qt 5.12 与 Qt 6.9（实验要求 Qt 6.9.2）。
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET   = QtLab1Calculator
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# 默认的构建目录规则：debug/release 子目录
CONFIG += debug_and_release
