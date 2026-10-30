QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TextEditor
TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 5) {
    CONFIG += c++17
} else {
    CONFIG += c++11
}

CONFIG += warn_on
CONFIG -= app_bundle

INCLUDEPATH += $$PWD

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    documentio.cpp \
    finddialog.cpp \
    replacedialog.cpp \
    aboutdialog.cpp

HEADERS += \
    mainwindow.h \
    codeeditor.h \
    documentio.h \
    finddialog.h \
    replacedialog.h \
    aboutdialog.h

FORMS += \
    mainwindow.ui \
    finddialog.ui \
    replacedialog.ui \
    aboutdialog.ui

RESOURCES += resources.qrc

# RC_ICONS 在含中文的工程路径下会被 windres 拒绝，程序图标改由 qrc + setWindowIcon 提供。
msvc: QMAKE_CXXFLAGS += /utf-8

QMAKE_TARGET_COMPANY = DGUT
QMAKE_TARGET_PRODUCT = Simple Text Editor
QMAKE_TARGET_DESCRIPTION = 简易文本编辑器
QMAKE_TARGET_COPYRIGHT = 黄俊豪

DESTDIR = $$PWD/bin
