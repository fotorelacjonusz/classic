#include "application.h"
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

	Application a(argc, argv);
	a.setApplicationName("fotorelacjonusz2");
	a.setOrganizationName("Kamil Ostaszewski software");

	QString locale = QLocale::system().name();
	QTranslator translator;

	if (locale == "pl_PL")
		translator.load("qt_pl", "/usr/share/qt4/translations") or translator.load("qt_pl");
	else
		translator.load("fotorelacjonusz_en_US");

	a.installTranslator(&translator);

	MainWindow w;
	w.setWindowTitle(a.applicationName());
	w.show();

	return a.exec();
}
