#-------------------------------------------------
#
# Project created by QtCreator 2014-02-01T19:32:45
#
#-------------------------------------------------

VERSION = 0.1.0

QT       += core gui widgets network

CONFIG += c++11

TARGET = yasem
CONFIG   += console
TEMPLATE = app
#CONFIG   -= app_bundle

TRANSLATIONS = lang/en_US.qm \
    lang/ru_RU.qm \
    lang/uk_UA.qm


SOURCES += main.cpp \
    pluginmanagerimpl.cpp \
    coreimpl.cpp \
    consolelogger.cpp \
    profilemanageimpl.cpp \
    networkimpl.cpp \
    pluginthread.cpp \
    loggercore.cpp \
    yasemapplication.cpp \
    diskinfo.cpp \
    stacktrace.cxx

HEADERS += \
    pluginmanager.h \
    pluginmanagerimpl.h \
    coreimpl.h \
    core.h \
    logger.h \
    consolelogger.h \
    mainwindow.h \
    plugindependency.h \
    profilemanager.h \
    profilemanageimpl.h \
    stbprofileplugin.h \
    datasource.h \
    networkimpl.h \
    stbprofile.h \
    customkeyevent.h \
    plugin.h \
    guiplugin.h \
    stbplugin.h \
    mediaplayerplugin.h \
    macros.h \
    enums.h  \
    datasourceplugin.h \
    browserplugin.h \
    pluginthread.h \
    mediasignalsender.h \
    profileconfig.h \
    core-network.h \
    loggercore.h \
    yasemapplication.h \
    diskinfo.h \
    webobjectinfo.h

include(../common.pri)
DESTDIR = $$DEFAULT_BIN_DIR

#unix:!mac{
#  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/
#  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/libs
#  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/plugins
#  QMAKE_RPATH=
#}

OTHER_FILES += \
    LICENSE \
    README.md


