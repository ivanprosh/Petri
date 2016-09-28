QT += core

CONFIG += c++11

TARGET = Lab2_3
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    Source/main.cpp

HEADERS += \
    Source/stdafx.h \
    Source/targetver.h \
    mythread.h \
    Source/mythread.h \
    Mod_Petri/zpetri.hxx \
    Mod_Petri/zpetri-env.hxx

deployment.path = $$OUT_PWD/
deployment.files += input.ini \

INSTALLS += deployment

DISTFILES += \
    input.ini
