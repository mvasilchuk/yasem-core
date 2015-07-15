#-------------------------------------------------
#
# Project created by QtCreator 2014-02-01T19:32:45
#
#-------------------------------------------------

VERSION = 0.1.0
TARGET = yasem
TEMPLATE = app
QMAKE_TARGET_COMPANY =
QMAKE_TARGET_DESCRIPTION = "Core module of YASEM project"
QMAKE_TARGET_PRODUCT = "yasem-core"
QMAKE_TARGET_COPYRIGHT = "2013-15 by Maxim Vasilchuk"
RC_ICONS = resources/logo.ico
ICON = resources/logo.ico

include($${top_srcdir}/common.pri)

QT += core widgets network
equals(QT_MAJOR_VERSION, 5): {
    QT -= gui
}

CONFIG -= console
QT.testlib.CONFIG -= console


#DEFINES += EXTRA_DEBUG_INFO #Set this flag to show extra information in logger output

TRANSLATIONS = lang/en_US.qm \
    lang/ru_RU.qm \
    lang/uk_UA.qm

SOURCES += main.cpp \
    pluginmanagerimpl.cpp \
    coreimpl.cpp \
    profilemanageimpl.cpp \
    networkimpl.cpp \
    pluginthread.cpp \
    loggercore.cpp \
    yasemapplication.cpp \
    stacktrace.cxx \
    profileconfigparserimpl.cpp \
    sambaimpl.cpp \
    mountpointinfo.cpp \
    networkstatisticsimpl.cpp \
    statisticsimpl.cpp \
    systemstatisticsimpl.cpp \
    configimpl.cpp

HEADERS += \
    pluginmanagerimpl.h \
    coreimpl.h \
    profilemanageimpl.h \
    networkimpl.h \
    pluginthread.h \
    loggercore.h \
    yasemapplication.h \
    profileconfigparserimpl.h \
    sambaimpl.h \
    mountpointinfo.h \
    networkstatisticsimpl.h \
    statisticsimpl.h \
    systemstatisticsimpl.h \
    configimpl.h

unix:!mac{
  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/
  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/libs
  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/plugins
  #QMAKE_RPATH +=
}

OTHER_FILES += \
    LICENSE \
    README.md

RESOURCES += \
    resources.qrc

