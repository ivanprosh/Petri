QT += core
QT -= gui

CONFIG += c++11

TARGET = Lab2
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    source/main.cpp \
    source/stdafx.cpp

HEADERS += \
    source/stdafx.h \
    source/targetver.h

DISTFILES += \
    input.txt \
    output.txt
