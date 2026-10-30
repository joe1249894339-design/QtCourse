QT += core gui widgets testlib

TARGET = tst_texteditor
TEMPLATE = app
CONFIG += console testcase c++17
CONFIG -= app_bundle

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/..
DESTDIR = $$PWD/bin

SOURCES += \
    tst_texteditor.cpp \
    ../mainwindow.cpp \
    ../codeeditor.cpp \
    ../documentio.cpp \
    ../finddialog.cpp \
    ../replacedialog.cpp \
    ../aboutdialog.cpp

HEADERS += \
    ../mainwindow.h \
    ../codeeditor.h \
    ../documentio.h \
    ../finddialog.h \
    ../replacedialog.h \
    ../aboutdialog.h

FORMS += \
    ../mainwindow.ui \
    ../finddialog.ui \
    ../replacedialog.ui \
    ../aboutdialog.ui

RESOURCES += ../resources.qrc
