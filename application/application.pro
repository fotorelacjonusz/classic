#-------------------------------------------------
#
# Project created by QtCreator 2012-08-23T10:07:44
#
#-------------------------------------------------

TEMPLATE = app
TARGET = fotorelacjonusz
macx:TARGET = Fotorelacjonusz
VERSION = 2.99.0

# Used in CFBundleIdentifier in Info.plist on Mac
QMAKE_TARGET_BUNDLE_PREFIX = org.forumpolskichwiezowcow

# MacOS icon set
ICON = appicon.icns

QT += core gui network script webchannel webengine webenginewidgets widgets xml xmlpatterns

# Enable C++11 explicitly, which should make proper stdlib available.
# Required to compile at least on OS X.
CONFIG += c++11

# Fotorelacjonusz relies on some Qt4 APIs which are now deprecated, but still
# available for legacy software.  This should be removed at some point.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=4

# Make version number accessible in the application via PROGRAM_VERSION macro
DEFINES += PROGRAM_VERSION=\\\"$$VERSION\\\"

QMAKE_CXXFLAGS_DEBUG += -rdynamic
QMAKE_LFLAGS_DEBUG += -g -rdynamic
QMAKE_CXXFLAGS_DEBUG -= -pipe

INCLUDEPATH += \
	../third-party/qtftp/include \
	../third-party/quazip

LIBS += \
	-L$$absolute_path(../target/qtftp/lib", $$_PRO_FILE_PWD_) \
	-L$$absolute_path("../target", $$_PRO_FILE_PWD_)

win32 {
	INCLUDEPATH += ../third-party/zlib
	LIBS += -lquazip1
	LIBS += -lQt5Ftp
}

unix {
	LIBS += -lquazip
	LIBS += -lQt5Ftp
}

TRANSLATIONS = fotorelacjonusz_en_US.ts

SOURCES += \
	main.cpp \
	networkcookiejar.cpp \
	messagehandler.cpp \
	exception.cpp \
	application.cpp \
	embeddedjavascript.cpp \
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
	uploaders/isanonuploader.cpp \
	uploaders/imgurloginuploader.cpp \
	uploaders/imguranonuploader.cpp \
	uploaders/ftpuploader.cpp \
	uploaders/abstractuploader.cpp \
	myexif/exifimageheader.cpp \
	myexif/exifrational.cpp \
	myexif/exifvalue.cpp \
	myexif/exififd.cpp \
	myexif/exifmarker.cpp

HEADERS += \
	networkcookiejar.h \
	messagehandler.h \
	exception.h \
	application.h \
	embeddedjavascript.h \
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
	uploaders/isanonuploader.h \
	uploaders/imgurloginuploader.h \
	uploaders/imguranonuploader.h \
	uploaders/ftpuploader.h \
	uploaders/abstractuploader.h \
	myexif/exifrational.h \
	myexif/exififd.h \
	myexif/exifvalue.h \
	myexif/exifmarker.h \
	myexif/exifimageheader.h

FORMS += \
	widgets/replydialog.ui \
	widgets/postwidget.ui \
	widgets/mainwindow.ui \
	widgets/gpxdialog.ui \
	settings/settingsdialog.ui \
	settings/osmlayerdialog.ui \
	uploaders/isloginuploader.ui \
	uploaders/iscodeuploader.ui \
	uploaders/isanonuploader.ui \
	uploaders/imgurloginuploader.ui \
	uploaders/imguranonuploader.ui \
	uploaders/ftpuploader.ui

RESOURCES += \
	../resource.qrc \
	../webscripts.qrc

# Prevent JavaScripts from being compiled to C++ sources.
QTQUICK_COMPILER_SKIPPED_RESOURCES += webscripts.qrc

OTHER_FILES += \
	../res/page.html
