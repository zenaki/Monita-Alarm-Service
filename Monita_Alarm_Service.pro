QT += core sql network websockets
QT -= gui

CONFIG += c++11

TARGET = Monita_Alarm_Service
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    worker.cpp \
    redis.cpp \
#    datasource.cpp \
    mysql.cpp \
    notification.cpp \
    monita_log.cpp \
    config.cpp

HEADERS += \
    worker.h \
    redis.h \
    utama.h \
#    datasource.h \
    mysql.h \
    notification.h \
    monita_log.h \
    config.h

INCLUDEPATH += /usr/local/include/hiredis
LIBS += -L/usr/local/lib -lhiredis
