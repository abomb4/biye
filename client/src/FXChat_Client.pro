#-------------------------------------------------
#
# Project created by QtCreator 2016-03-28T19:35:05
#
#-------------------------------------------------

QT       += core gui network sql

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FXChat_Client
TEMPLATE = app


SOURCES += main.cpp \
    ui/login.cpp \
    ui/mainwindow.cpp \
    ui/settings.cpp \
    ui/chatingwindow.cpp \
    fxclient.cpp \
    fxmessage.cpp \
    linearmemorypool.cpp \
    fxconnection.cpp \
    config.cpp \
    clientdb.cpp

HEADERS  += \
    ui/login.h \
    ui/mainwindow.h \
    ui/settings.h \
    ui/chatingwindow.h \
    fxclient.h \
    fxmessage.h \
    linearmemorypool.h \
    memorypool.h \
    fxconnection.h \
    config.h \
    clientdb.h \
    structs/user.h \
    structs/chatlog.h \
    structs/department.h

FORMS    += ui/login.ui \
    ui/mainwindow.ui \
    ui/settings.ui \
    ui/chatingwindow.ui

DISTFILES += \
    default.cfg

RESOURCES += \
    icons.qrc
