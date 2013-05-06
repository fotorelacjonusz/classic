#-------------------------------------------------
#
# Project created by QtCreator 2012-08-23T10:07:44
#
#-------------------------------------------------

QT       += core gui network webkit script xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fotorelacjonusz2
TEMPLATE = app


SOURCES += main.cpp \
    networkcookiejar.cpp \
    exception.cpp \
    application.cpp \
    widgets/simpleimage.cpp \
    widgets/selectablewidget.cpp \
    widgets/replydialog.cpp \
    widgets/recentthreadsmenu.cpp \
    widgets/postwidget.cpp \
    widgets/mainwindow.cpp \
    widgets/lineedit.cpp \
    widgets/imagewidget.cpp \
    widgets/imagemanipulation.cpp \
    widgets/imagelabel.cpp \
    widgets/colormanipulationtoolbar.cpp \
    widgets/arrowwidget.cpp \
    widgets/abstractimage.cpp \
    widgets/aboutdialog.cpp \
    settings/settingsmanager.cpp \
    settings/settingsdialog.cpp \
    settings/overlaylist.cpp \
    settings/overlay.cpp \
    settings/osmlayerdialog.cpp \
    settings/displayslider.cpp \
    downloaders/tilesdownloader.cpp \
    downloaders/throttlednetworkmanager.cpp \
    downloaders/textballoon.cpp \
    downloaders/gpsdata.cpp \
    downloaders/googlemapsdownloader.cpp \
    downloaders/abstractmapdownloader.cpp \
    uploaders/uploaderfactory.cpp \
    uploaders/networktransactionquery.cpp \
    uploaders/networktransactionmultipart.cpp \
    uploaders/networktransaction.cpp \
    uploaders/jsonobject.cpp \
    uploaders/isloginuploader.cpp \
    uploaders/iscodeuploader.cpp \
    uploaders/isanonyuploader.cpp \
    uploaders/imgurloginuploader.cpp \
    uploaders/imguranonyuploader.cpp \
    uploaders/ftpuploader.cpp \
    uploaders/abstractuploader.cpp

HEADERS += \
    networkcookiejar.h \
    exception.h \
    application.h \
    widgets/simpleimage.h \
    widgets/selectablewidget.h \
    widgets/replydialog.h \
    widgets/recentthreadsmenu.h \
    widgets/postwidget.h \
    widgets/mainwindow.h \
    widgets/lineedit.h \
    widgets/imagewidget.h \
    widgets/imagemanipulation.h \
    widgets/imagelabel.h \
    widgets/colormanipulationtoolbar.h \
    widgets/arrowwidget.h \
    widgets/abstractimage.h \
    widgets/aboutdialog.h \
    settings/settingsmanager.h \
    settings/settingsdialog.h \
    settings/overlaylist.h \
    settings/overlay.h \
    settings/osmlayerdialog.h \
    settings/displayslider.h \
    downloaders/tilesdownloader.h \
    downloaders/throttlednetworkmanager.h \
    downloaders/textballoon.h \
    downloaders/gpsdata.h \
    downloaders/googlemapsdownloader.h \
    downloaders/abstractmapdownloader.h \
    uploaders/uploaderfactory.h \
    uploaders/networktransactionquery.h \
    uploaders/networktransactionmultipart.h \
    uploaders/networktransaction.h \
    uploaders/jsonobject.h \
    uploaders/isloginuploader.h \
    uploaders/iscodeuploader.h \
    uploaders/isanonyuploader.h \
    uploaders/imgurloginuploader.h \
    uploaders/imguranonyuploader.h \
    uploaders/ftpuploader.h \
    uploaders/abstractuploader.h

FORMS += \
    widgets/replydialog.ui \
    widgets/postwidget.ui \
    widgets/mainwindow.ui \
    widgets/aboutdialog.ui \
    settings/settingsdialog.ui \
    settings/osmlayerdialog.ui \
    uploaders/isloginuploader.ui \
    uploaders/iscodeuploader.ui \
    uploaders/isanonyuploader.ui \
    uploaders/imgurloginuploader.ui \
    uploaders/imguranonyuploader.ui \
    uploaders/ftpuploader.ui



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

