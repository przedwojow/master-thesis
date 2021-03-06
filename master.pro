#-------------------------------------------------
#
# Project created by QtCreator 2016-03-19T11:44:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = master
TEMPLATE = app

INCLUDEPATH+=/usr/local/qwt-6.1.2/include
LIBS+=-L/usr/local/qwt-6.1.2/lib -lqwt

SOURCES += main.cpp\
        mainwindow.cpp \
    console/console.cpp \
    console/settingsdialog.cpp \
    simulation/signal.cpp \
    simulation/simulation.cpp \
    sensors/receiver.cpp \
    sensors/obstacle.cpp \
    sensors/emitter.cpp \
    sensors/object.cpp \
    matlab/matlabexporter.cpp \
    algorithm/algorithm.cpp \
    simulation/point.cpp \
    simsettingsdialog.cpp

HEADERS  += mainwindow.h \
    constants.h \
    console/console.h \
    console/settingsdialog.h \
    simulation/signal.h \
    simulation/simulation.h \
    sensors/receiver.h \
    sensors/obstacle.h \
    sensors/object.h \
    sensors/emitter.h \
    matlab/matlabexporter.h \
    algorithm/algorithm.h \
    simulation/point.h \
    simsettingsdialog.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    simsettingsdialog.ui
CONFIG += c++11
