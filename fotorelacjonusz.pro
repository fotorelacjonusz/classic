#-------------------------------------------------
#
# Project created by QtCreator 2012-08-23T10:07:44
#
#-------------------------------------------------

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=4

QT       += core gui network webkit webkitwidgets script widgets xml xmlpatterns

TARGET = fotorelacjonusz
TEMPLATE = app

SOURCES += src/main.cpp \
    src/networkcookiejar.cpp \
    src/messagehandler.cpp \
    src/exception.cpp \
    src/application.cpp \
    src/widgets/threadedvalidator.cpp \
    src/widgets/selectablewidget.cpp \
    src/widgets/replydialog.cpp \
    src/widgets/recentthreadsmenu.cpp \
    src/widgets/questionbox.cpp \
    src/widgets/progresscontainer.cpp \
    src/widgets/postwidget.cpp \
    src/widgets/ntppacket.cpp \
    src/widgets/ntpclient.cpp \
    src/widgets/mainwindow.cpp \
    src/widgets/lineedit.cpp \
    src/widgets/imagewidget.cpp \
    src/widgets/imagemanipulation.cpp \
    src/widgets/imagelabel.cpp \
    src/widgets/gpxdialog.cpp \
    src/widgets/colormanipulationtoolbar.cpp \
    src/widgets/arrowwidget.cpp \
    src/widgets/abstractimage.cpp \
    src/settings/settingsmanager.cpp \
    src/settings/settingsdialog.cpp \
    src/settings/overlaylist.cpp \
    src/settings/overlayimage.cpp \
    src/settings/overlay.cpp \
    src/settings/osmlayerdialog.cpp \
    src/settings/displayslider.cpp \
    src/downloaders/tilesdownloader.cpp \
    src/downloaders/throttlednetworkmanager.cpp \
    src/downloaders/gpsdata.cpp \
    src/downloaders/googlemapsdownloader.cpp \
    src/downloaders/geomap.cpp \
    src/downloaders/abstractmapdownloader.cpp \
    src/uploaders/uploaderfactory.cpp \
    src/uploaders/networktransactionquery.cpp \
    src/uploaders/networktransactionmultipart.cpp \
    src/uploaders/networktransaction.cpp \
    src/uploaders/jsonobject.cpp \
    src/uploaders/isloginuploader.cpp \
    src/uploaders/iscodeuploader.cpp \
    src/uploaders/isanonyuploader.cpp \
    src/uploaders/imgurloginuploader.cpp \
    src/uploaders/imguranonyuploader.cpp \
    src/uploaders/ftpuploader.cpp \
    src/uploaders/abstractuploader.cpp \
    src/myexif/exifimageheader.cpp \
    src/myexif/exifrational.cpp \
    src/myexif/exifvalue.cpp \
    src/myexif/exififd.cpp \
    src/myexif/exifmarker.cpp

HEADERS += \
    src/networkcookiejar.h \
    src/messagehandler.h \
    src/exception.h \
    src/application.h \
    src/widgets/threadedvalidator.h \
    src/widgets/selectablewidget.h \
    src/widgets/replydialog.h \
    src/widgets/recentthreadsmenu.h \
    src/widgets/questionbox.h \
    src/widgets/progresscontainer.h \
    src/widgets/postwidget.h \
    src/widgets/ntppacket.h \
    src/widgets/ntpclient.h \
    src/widgets/mainwindow.h \
    src/widgets/lineedit.h \
    src/widgets/imagewidget.h \
    src/widgets/imagemanipulation.h \
    src/widgets/imagelabel.h \
    src/widgets/gpxdialog.h \
    src/widgets/colormanipulationtoolbar.h \
    src/widgets/arrowwidget.h \
    src/widgets/abstractimage.h \
    src/settings/settingsmanager.h \
    src/settings/settingsdialog.h \
    src/settings/overlaylist.h \
    src/settings/overlayimage.h \
    src/settings/overlay.h \
    src/settings/osmlayerdialog.h \
    src/settings/displayslider.h \
    src/downloaders/tilesdownloader.h \
    src/downloaders/throttlednetworkmanager.h \
    src/downloaders/gpsdata.h \
    src/downloaders/googlemapsdownloader.h \
    src/downloaders/geomap.h \
    src/downloaders/abstractmapdownloader.h \
    src/uploaders/uploaderfactory.h \
    src/uploaders/networktransactionquery.h \
    src/uploaders/networktransactionmultipart.h \
    src/uploaders/networktransaction.h \
    src/uploaders/jsonobject.h \
    src/uploaders/isloginuploader.h \
    src/uploaders/iscodeuploader.h \
    src/uploaders/isanonyuploader.h \
    src/uploaders/imgurloginuploader.h \
    src/uploaders/imguranonyuploader.h \
    src/uploaders/ftpuploader.h \
    src/uploaders/abstractuploader.h \
    src/myexif/exifrational.h \
    src/myexif/exififd.h \
    src/myexif/exifvalue.h \
    src/myexif/exifmarker.h \
    src/myexif/exifimageheader.h

FORMS += \
    src/widgets/replydialog.ui \
    src/widgets/postwidget.ui \
    src/widgets/mainwindow.ui \
    src/widgets/gpxdialog.ui \
    src/settings/settingsdialog.ui \
    src/settings/osmlayerdialog.ui \
    src/uploaders/isloginuploader.ui \
    src/uploaders/iscodeuploader.ui \
    src/uploaders/isanonyuploader.ui \
    src/uploaders/imgurloginuploader.ui \
    src/uploaders/imguranonyuploader.ui \
    src/uploaders/ftpuploader.ui

RESOURCES += \
    resource.qrc

LIBS += \
    -L../quazip

INCLUDEPATH += third-party/quazip/quazip src

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

QMAKE_CXXFLAGS_DEBUG += -rdynamic
QMAKE_LFLAGS_DEBUG += -g -rdynamic
QMAKE_CXXFLAGS_DEBUG -= -pipe
