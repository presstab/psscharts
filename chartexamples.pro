#-------------------------------------------------
#
# Project created by QtCreator 2020-02-25T18:20:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chartexamples
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        chartexamples/main.cpp \
        chartexamples/mainwindow.cpp \
        src/barchart.cpp \
        src/candlestickchart.cpp \
        src/legendwidget.cpp \
        src/linechart.cpp \
        src/chart.cpp \
        src/piechart.cpp \
        src/stringutil.cpp \
        src/mousedisplay.cpp

HEADERS += \
        chartexamples/mainwindow.h \
        src/barchart.h \
        src/candlestickchart.h \
        src/legendwidget.h \
        src/linechart.h \
        src/chart.h \
        src/piechart.h \
        src/stringutil.h \
        src/axislabelsettings.h \
        src/mousedisplay.h

FORMS += \
        chartexamples/mainwindow.ui \
        src/legendwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
