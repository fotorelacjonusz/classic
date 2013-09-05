#include "application.h"
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include <QPixmapCache>
#include <cstdio>
#include "widgets/mainwindow.h"
#include "messagehandler.h"

#ifdef Q_OS_UNIX

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) 
{
	void *array[10];
	size_t size = backtrace(array, 10);
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}

#endif

int main(int argc, char *argv[])
{

#ifdef Q_OS_UNIX
	signal(SIGSEGV, handler);
#endif
	
	const char *version = "2.5.5";
	if (argc >= 2 && QString(argv[1]) == "-v")
	{
		fprintf(stdout, "%s\n", version);
		return 0;
	}
	
	qInstallMsgHandler(messageHandler);
	
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	
	QPixmapCache::setCacheLimit(524288);

	Application a(argc, argv);
	a.setApplicationName("fotorelacjonusz");
	a.setOrganizationName("Kamil Ostaszewski software");
	a.setApplicationVersion(version);

	QString locale = QLocale::system().name();
	QTranslator translator;

	qDebug() << "Locale:" << locale;
	if (locale == "pl_PL")
		qDebug() << "Loading qt_pl:" << (translator.load("qt_pl", "/usr/share/qt4/translations") or translator.load("qt_pl"));
	else
		qDebug() << "Loading :/fotorelacjonusz_en_US:" << translator.load(":/fotorelacjonusz_en_US");

	a.installTranslator(&translator);

	MainWindow w;
	w.setWindowTitle(a.applicationName());
	w.show();

	return a.exec();
}
