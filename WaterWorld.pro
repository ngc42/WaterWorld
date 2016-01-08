TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp

INCLUDEPATH += /usr/include
LIBS += -L/usr/lib/x86_64-linux-gnu/
CONFIG(release, debug|release): LIBS += -lsfml-audio -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system
CONFIG(debug, debug|release): LIBS += -lsfml-audio-d -lsfml-graphics-d -lsfml-network-d -lsfml-window-d -lsfml-system-d

HEADERS += \
    waterobject.h \
    isle.h \
    ship.h \
    screens.h \
    manager.h
