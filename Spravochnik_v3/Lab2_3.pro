QT += core

CONFIG += c++11

TARGET = Lab2_3
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp

HEADERS += \
    mythread.h \
    Mod_Petri/zpetri.hxx \
    Mod_Petri/zpetri-env.hxx \
    stdafx.h \
    targetver.h \
    mypetri.h \
    note.h

deployment.path = $$OUT_PWD/
deployment.files += input.ini \

INSTALLS += deployment

DISTFILES += \
    input.ini
