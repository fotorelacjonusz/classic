#-------------------------------------------------
#
# Project created by QtCreator 2012-08-23T10:07:44
#
#-------------------------------------------------

QT       += core gui network webkit script xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fotorelacjonusz2
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    imagewidget.cpp \
    settingsdialog.cpp \
    aboutdialog.cpp \
    replydialog.cpp \
    postwidget.cpp \
    arrowwidget.cpp \
    imagelabel.cpp \
    abstractuploader.cpp \
    isanonyuploader.cpp \
    iscodeuploader.cpp \
    isloginuploader.cpp \
    ftpuploader.cpp \
    gpsdata.cpp \
    abstractimage.cpp \
    simpleimage.cpp \
    settingsmanager.cpp \
    imagemanipulation.cpp \
    colormanipulationtoolbar.cpp \
    exception.cpp \
    application.cpp \
    displayslider.cpp \
    networkcookiejar.cpp \
    overlay.cpp \
    imguranonyuploader.cpp \
    imgurloginuploader.cpp \
    jsonobject.cpp \
    networktransaction.cpp \
    networktransactionquery.cpp \
    networktransactionmultipart.cpp \
    overlaylist.cpp \
    uploaderfactory.cpp \
    osmlayerdialog.cpp \
    tilesdownloader.cpp \
    textballoon.cpp \
    googlemapsdownloader.cpp \
    throttlednetworkmanager.cpp \
    abstractmapdownloader.cpp \
    lineedit.cpp \
    selectablewidget.cpp

HEADERS  += mainwindow.h \
    imagewidget.h \
    settingsdialog.h \
    aboutdialog.h \
    replydialog.h \
    postwidget.h \
    arrowwidget.h \
    imagelabel.h \
    abstractuploader.h \
    isanonyuploader.h \
    iscodeuploader.h \
    isloginuploader.h \
    ftpuploader.h \
    gpsdata.h \
    abstractimage.h \
    simpleimage.h \
    settingsmanager.h \
    imagemanipulation.h \
    colormanipulationtoolbar.h \
    exception.h \
    application.h \
    displayslider.h \
    networkcookiejar.h \
    overlay.h \
    imguranonyuploader.h \
    imgurloginuploader.h \
    jsonobject.h \
    networktransaction.h \
    networktransactionquery.h \
    networktransactionmultipart.h \
    overlaylist.h \
    uploaderfactory.h \
    secrets.h \
    osmlayerdialog.h \
    tilesdownloader.h \
    textballoon.h \
    googlemapsdownloader.h \
    throttlednetworkmanager.h \
    abstractmapdownloader.h \
    lineedit.h \
    selectablewidget.h

FORMS    += mainwindow.ui \
    settingsdialog.ui \
    aboutdialog.ui \
    replydialog.ui \
    postwidget.ui \
    isanonyuploader.ui \
    iscodeuploader.ui \
    isloginuploader.ui \
    ftpuploader.ui \
    imguranonyuploader.ui \
    imgurloginuploader.ui \
    osmlayerdialog.ui

RESOURCES += \
    resource.qrc

LIBS += \
    -L../quazip

INCLUDEPATH += third-party/quazip/quazip

win32 {
	LIBS += -lquazip1
    INCLUDEPATH += third-party/zlib
}

unix {
    LIBS += -lquazip
}

OTHER_FILES += \
    res/page.html

TRANSLATIONS = fotorelacjonusz_en_US.ts

