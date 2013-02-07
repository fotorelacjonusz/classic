#-------------------------------------------------
#
# Project created by QtCreator 2012-08-06T21:20:49
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = exif
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

win32 {
    LIBS += -lexiv2-11
	INCLUDEPATH += \
		../third-party
}
unix {
    LIBS += -lexiv2
}

OTHER_FILES +=

HEADERS += \
    license.h
