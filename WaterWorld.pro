#-------------------------------------------------
#
# Project created by QtCreator 2016-01-22T09:47:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WaterWorld
TEMPLATE = app

CONFIG += console
CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp \
    universescene.cpp \
    minimapview.cpp \
    universeview.cpp \
    universe.cpp \
    waterobject.cpp \
    isle.cpp \
    ship.cpp \
    shiplistitem.cpp \
    strategy.cpp \
    overviewdialog.cpp \
    graphicspathitem.cpp \
    pathlistitem.cpp

HEADERS  += mainwindow.h \
    universescene.h \
    minimapview.h \
    universeview.h \
    universe.h \
    waterobject.h \
    isle.h \
    ship.h \
    shiplistitem.h \
    strategy.h \
    overviewdialog.h \
    graphicspathitem.h \
    pathlistitem.h

FORMS    += mainwindow.ui \
    waterobjectinfo.ui \
    overviewdialog.ui
