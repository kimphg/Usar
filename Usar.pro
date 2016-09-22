#-------------------------------------------------
#
# Project created by QtCreator 2016-01-05T17:33:34
# last modified 16.5
#-------------------------------------------------

QT       += core gui
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RadarSimulator_1.2
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
        vnmap.cpp \
    Config.cpp \
    C_radar_data.cpp \
    c_arpa_data.cpp \
    qhoversensitivebutton.cpp

HEADERS  += mainwindow.h \
        vnmap.h \
    Config.h \
    C_radar_data.h \
    c_arpa_data.h \
    ui_mainwindow.h \
    qhoversensitivebutton.h


win32:LIBS += -L$$PWD/armadilloWin32/lib_winx86/ -lblas_win32_MT
win32:LIBS += -L$$PWD/armadilloWin32/lib_winx86/ -llapack_win32_MT
FORMS    += mainwindow.ui
DISTFILES += \
    appIcon.rc
win32:RC_FILE += appIcon.rc
