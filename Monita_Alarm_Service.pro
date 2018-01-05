QT += core sql network websockets script
QT -= gui

CONFIG += c++11

TARGET = monita-alarm-service
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp                     \
    controller/worker.cpp               \
    modul/redis.cpp                     \
#    controller/datasource.cpp          \
    modul/mysql.cpp                     \
    controller/notification.cpp         \
    modul/monita_log.cpp                \
    util/config.cpp                     \
    controller/scheduler.cpp            \
    3rdparty/qcron/src/holiday.cpp      \
    3rdparty/qcron/src/qcron.cpp        \
    3rdparty/qcron/src/qcronfield.cpp   \
    3rdparty/qcron/src/qcronnode.cpp    \
#    controller/processalarm.cpp        \
    modul/smtp.cpp
#    3rdparty/SmtpClient/src/emailaddress.cpp \
#    3rdparty/SmtpClient/src/mimeattachment.cpp \
#    3rdparty/SmtpClient/src/mimecontentformatter.cpp \
#    3rdparty/SmtpClient/src/mimefile.cpp \
#    3rdparty/SmtpClient/src/mimehtml.cpp \
#    3rdparty/SmtpClient/src/mimeinlinefile.cpp \
#    3rdparty/SmtpClient/src/mimemessage.cpp \
#    3rdparty/SmtpClient/src/mimemultipart.cpp \
#    3rdparty/SmtpClient/src/mimepart.cpp \
#    3rdparty/SmtpClient/src/mimetext.cpp \
#    3rdparty/SmtpClient/src/quotedprintable.cpp \
#    3rdparty/SmtpClient/src/smtpclient.cpp

HEADERS +=                              \
    controller/worker.h                 \
    modul/redis.h                       \
    util/utama.h                        \
#    controller/datasource.h            \
    modul/mysql.h                       \
    controller/notification.h           \
    modul/monita_log.h                  \
    util/config.h                       \
    controller/scheduler.h              \
    3rdparty/qcron/src/holiday.hpp      \
    3rdparty/qcron/src/qcron.hpp        \
    3rdparty/qcron/src/qcronfield.hpp   \
    3rdparty/qcron/src/qcronnode.hpp    \
#    controller/processalarm.h          \
    modul/smtp.h
#    3rdparty/SmtpClient/src/emailaddress.h \
#    3rdparty/SmtpClient/src/mimeattachment.h \
#    3rdparty/SmtpClient/src/mimecontentformatter.h \
#    3rdparty/SmtpClient/src/mimefile.h \
#    3rdparty/SmtpClient/src/mimehtml.h \
#    3rdparty/SmtpClient/src/mimeinlinefile.h \
#    3rdparty/SmtpClient/src/mimemessage.h \
#    3rdparty/SmtpClient/src/mimemultipart.h \
#    3rdparty/SmtpClient/src/mimepart.h \
#    3rdparty/SmtpClient/src/mimetext.h \
#    3rdparty/SmtpClient/src/quotedprintable.h \
#    3rdparty/SmtpClient/src/smtpclient.h \
#    3rdparty/SmtpClient/src/smtpexports.h \
#    3rdparty/SmtpClient/src/SmtpMime


INCLUDEPATH += /usr/local/include/hiredis
LIBS += -L/usr/local/lib -lhiredis

## Location of SMTP Library
#SMTP_LIBRARY_LOCATION = $$PWD/3rdparty/SmtpClient/src

#win32:CONFIG(release, debug|release): LIBS += -L$$SMTP_LIBRARY_LOCATION/release/ -lSMTPEmail
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$SMTP_LIBRARY_LOCATION/debug/ -lSMTPEmail
#else:unix: LIBS += -L$$SMTP_LIBRARY_LOCATION -lSMTPEmail

#INCLUDEPATH += $$SMTP_LIBRARY_LOCATION
#DEPENDPATH += $$SMTP_LIBRARY_LOCATION

INCLUDEPATH += $$PWD/3rdparty/qcron
DEPENDPATH += $$PWD/3rdparty/qcron
