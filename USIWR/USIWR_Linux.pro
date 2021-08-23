#-------------------------------------------------
#
# Project created by QtCreator 2019-07-19T15:45:29
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = USIWR_Linux
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    LinuxHIDFeatureCtrl.cpp \
    USIWRArgParser.cpp

HEADERS += \
    LinuxHIDFeatureCtrl.h \
    USIWRArgParser.h
