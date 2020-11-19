#-------------------------------------------------
#
# Project created by QtCreator 2012-08-23T10:07:44
#
#-------------------------------------------------

TEMPLATE = app
TARGET = fotorelacjonusz
macx:TARGET = Fotorelacjonusz
VERSION = 3.2.0

# Used in CFBundleIdentifier in Info.plist on Mac
QMAKE_TARGET_BUNDLE_PREFIX = org.skalee.fotorelacjonusz

# MacOS icon set
ICON = appicon.icns

QT += core gui network websockets widgets xml xmlpatterns

# Enable C++11 explicitly, which should make proper stdlib available.
# Required to compile at least on OS X.
CONFIG += c++11

# Fotorelacjonusz relies on some Qt4 APIs which are now deprecated, but still
# available for legacy software.  This should be removed at some point.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=4

# Make version number accessible in the application via PROGRAM_VERSION macro
DEFINES += PROGRAM_VERSION=\\\"$$VERSION\\\"

QMAKE_LFLAGS_DEBUG += -g -rdynamic

INCLUDEPATH += \
	src \
	third-party/qtftp/include \
	third-party/quazip

LIBS += \
	-L$$absolute_path("third-party/qtftp/lib", $$_PRO_FILE_PWD_) \
	-L$$absolute_path("third-party/quazip/quazip", $$_PRO_FILE_PWD_)

win32 {
	INCLUDEPATH += third-party/zlib
	LIBS += -lquazip1
	LIBS += -lQt5Ftp
}

unix {
	LIBS += -lquazip
	LIBS += -lQt5Ftp
}

TRANSLATIONS = fotorelacjonusz_en_US.ts

SOURCES += \
	src/main.cpp \
	src/networkcookiejar.cpp \
	src/messagehandler.cpp \
	src/exception.cpp \
	src/application.cpp \
	src/uploaders/imgurauthenticator.cpp \
	src/uploaders/imgurresponse.cpp \
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
	src/uploaders/isloginuploader.cpp \
	src/uploaders/iscodeuploader.cpp \
	src/uploaders/isanonuploader.cpp \
	src/uploaders/imgurloginuploader.cpp \
	src/uploaders/imguranonuploader.cpp \
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
	src/uploaders/imgurauthenticator.h \
	src/uploaders/imgurresponse.h \
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
	src/uploaders/isloginuploader.h \
	src/uploaders/iscodeuploader.h \
	src/uploaders/isanonuploader.h \
	src/uploaders/imgurloginuploader.h \
	src/uploaders/imguranonuploader.h \
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
	src/uploaders/isanonuploader.ui \
	src/uploaders/imgurloginuploader.ui \
	src/uploaders/imguranonuploader.ui \
	src/uploaders/ftpuploader.ui

RESOURCES += \
	resource.qrc

OTHER_FILES += \
	res/page.html
