#-------------------------------------------------
#
# Project created by QtCreator 2018-08-22T14:00:41
#
#-------------------------------------------------

QT       += core

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG +=console

TARGET = USIWR
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        USIWRArgParser.cpp \

win32:SOURCES += \
    search/USBDevNode.cpp \
    search/WinDevSearcher.cpp \
    search/WinVerHelper.cpp \
    search/hid.cpp \
    WinHIDFeatureCtrl.cpp
linux:SOURCES +=

HEADERS += \
    USIWRArgParser.h \

win32:HEADERS += \
    search/AUString.h \
    search/Convert.h \
    search/EndianConvert.h \
    search/SiSInt.h \
    search/SysDefine.h \
    search/UnderlyingDevDefine.h \
    search/UnderlyingDevKey.h \
    search/USBDevNode.h \
    search/WinDevSearcher.h \
    search/WinVerHelper.h \
    search/hid.h \
    WinHIDFeatureCtrl.h

linux:HEADERS +=

win32:INCLUDEPATH += $$PWD/search
win32:LIBS += -L$$PWD/ -lhid -lsetupapi

win32: LIBS += "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64\setupapi.lib"
win32: LIBS += "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64\hid.lib"
win32: LIBS += "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64\Advapi32.lib"

