#-------------------------------------------------
#
# Project created by QtCreator 2012-08-23T10:07:44
#
#-------------------------------------------------

QT       += core gui network webkit script xml xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fotorelacjonusz
TEMPLATE = app

SOURCES += main.cpp \
    networkcookiejar.cpp \
    messagehandler.cpp \
    exception.cpp \
    application.cpp \
    widgets/threadedvalidator.cpp \
    widgets/selectablewidget.cpp \
    widgets/replydialog.cpp \
    widgets/recentthreadsmenu.cpp \
    widgets/questionbox.cpp \
    widgets/progresscontainer.cpp \
    widgets/postwidget.cpp \
    widgets/ntppacket.cpp \
    widgets/ntpclient.cpp \
    widgets/mainwindow.cpp \
    widgets/lineedit.cpp \
    widgets/imagewidget.cpp \
    widgets/imagemanipulation.cpp \
    widgets/imagelabel.cpp \
    widgets/gpxdialog.cpp \
    widgets/colormanipulationtoolbar.cpp \
    widgets/arrowwidget.cpp \
    widgets/abstractimage.cpp \
    settings/settingsmanager.cpp \
    settings/settingsdialog.cpp \
    settings/overlaylist.cpp \
    settings/overlayimage.cpp \
    settings/overlay.cpp \
    settings/osmlayerdialog.cpp \
    settings/displayslider.cpp \
    downloaders/tilesdownloader.cpp \
    downloaders/throttlednetworkmanager.cpp \
    downloaders/gpsdata.cpp \
    downloaders/googlemapsdownloader.cpp \
    downloaders/geomap.cpp \
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
    uploaders/abstractuploader.cpp \
    myexif/exifimageheader.cpp \
    myexif/exifrational.cpp \
    myexif/exifvalue.cpp \
    myexif/exififd.cpp \
    myexif/exifmarker.cpp \
    myexif/exifexception.cpp

HEADERS += \
    networkcookiejar.h \
    messagehandler.h \
    exception.h \
    application.h \
    widgets/threadedvalidator.h \
    widgets/selectablewidget.h \
    widgets/replydialog.h \
    widgets/recentthreadsmenu.h \
    widgets/questionbox.h \
    widgets/progresscontainer.h \
    widgets/postwidget.h \
    widgets/ntppacket.h \
    widgets/ntpclient.h \
    widgets/mainwindow.h \
    widgets/lineedit.h \
    widgets/imagewidget.h \
    widgets/imagemanipulation.h \
    widgets/imagelabel.h \
    widgets/gpxdialog.h \
    widgets/colormanipulationtoolbar.h \
    widgets/arrowwidget.h \
    widgets/abstractimage.h \
    settings/settingsmanager.h \
    settings/settingsdialog.h \
    settings/overlaylist.h \
    settings/overlayimage.h \
    settings/overlay.h \
    settings/osmlayerdialog.h \
    settings/displayslider.h \
    downloaders/tilesdownloader.h \
    downloaders/throttlednetworkmanager.h \
    downloaders/gpsdata.h \
    downloaders/googlemapsdownloader.h \
    downloaders/geomap.h \
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
    uploaders/abstractuploader.h \
    myexif/exifrational.h \
    myexif/exififd.h \
    myexif/exifvalue.h \
    myexif/exifmarker.h \
    myexif/exifimageheader.h \
    myexif/exifexception.h

FORMS += \
    widgets/replydialog.ui \
    widgets/postwidget.ui \
    widgets/mainwindow.ui \
    widgets/gpxdialog.ui \
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

