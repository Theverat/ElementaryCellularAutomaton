#-------------------------------------------------
#
# Project created by QtCreator 2014-05-03T23:33:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CellularAutomaton
TEMPLATE = app

QMAKE_CXXFLAGS += -fopenmp -std=c++11
LIBS += -fopenmp

SOURCES += main.cpp\
        mainwindow.cpp \
    eca.cpp

HEADERS  += mainwindow.h \
    eca.h

FORMS    += mainwindow.ui
