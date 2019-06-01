#define _SYS_SYSMACROS_H // conflicts with major() and minor() 

#include "application.h"
#include "embeddedjavascript.h"
#include "exception.h"
#include "widgets/mainwindow.h"
#include <QCursor>
#include <QDebug>
#include <QLocale>
#include <QStringList>
#include <QTranslator>

Application::Application(int &argc, char **argv):
	QApplication(argc, argv)
{
	setApplicationName("Fotorelacjonusz");
	setOrganizationName("FPW Community");
	setApplicationVersion(PROGRAM_VERSION);

	#if defined(Q_OS_MACOS)
	this->setAttribute(Qt::AA_DontShowIconsInMenus, true);
	#endif

	// Set locale and translator
	QString locale = QLocale::system().name();
	QTranslator translator;

	qDebug() << "Locale:" << locale;
	if (locale == "pl_PL")
		qDebug() << "Loading qt_pl:" << (translator.load("qt_pl", "/usr/share/qt4/translations") or translator.load("qt_pl"));
	else
		qDebug() << "Loading :/fotorelacjonusz_en_US:" << translator.load(":/fotorelacjonusz_en_US");

	installTranslator(&translator);

	// Insert JavaScripts for WebEngine
	EmbeddedJavascript::insertIntoProfile(":/qtwebchannel/qwebchannel.js");
	EmbeddedJavascript::insertIntoProfile(":/src/web/ssc.js");
}

/**
 * @brief Shows main window and enters main event loop.
 * @return program's exit code
 * @see QCoreApplication::exec
 */
int Application::showWindowAndExec()
{
	MainWindow w;
	w.setWindowTitle(applicationName());
	w.show();
	return exec();
}

QString Application::applicationSettingsName()
{
	return applicationName() + QString::number(applicationVersionNumber().majorVersion());
}

QString Application::applicationNameAndVersion()
{
	return applicationName() + " " + applicationVersion();
}

/**
 * @brief Basically applicationVersion(), but as a QVersionNumber.
 * @return application version number
 * @see QVersionNumber, QCoreApplication::applicationVersion()
 */
QVersionNumber Application::applicationVersionNumber()
{
	return QVersionNumber::fromString(applicationVersion());
}

void Application::busy()
{
	qApp->processEvents();
	setOverrideCursor(Qt::WaitCursor);
}

void Application::idle()
{
	while (overrideCursor())
		restoreOverrideCursor();
	qApp->processEvents();
}

bool Application::notify(QObject *object, QEvent *event)
{
	try
	{
		return QApplication::notify(object, event);
	}
	catch (const Exception &e)
	{
		e.showMessage(nullptr);
	}
	return false;
}

Version::Version(QString version):
	major (version.section('.', 0, 0).toInt()),
	format(version.section('.', 1, 1).toInt()),
	minor (version.section('.', 2, 2).toInt())
{
	Q_ASSERT(QRegExp("\\d+\\.\\d+\\.\\d+").exactMatch(version));
}

bool Version::isPhrCompatible(const Version &other)
{
	return major == other.major && format == other.format;
}
