#define _SYS_SYSMACROS_H // conflicts with major() and minor()

#include "application.h"
#include "exception.h"
#include "widgets/mainwindow.h"
#include <QCursor>
#include <QDebug>
#include <QLocale>
#include <QStringList>

Application::Application(int &argc, char **argv):
	QApplication(argc, argv)
{
	setApplicationName("Fotorelacjonusz");
	setOrganizationName("FPW Community");
	setApplicationVersion(PROGRAM_VERSION);

	#if defined(Q_OS_MACOS)
	this->setAttribute(Qt::AA_DontShowIconsInMenus, true);
	#endif

	setUpTranslations();
}

void Application::setUpTranslations()
{
	QLocale locale = QLocale::system();
	qDebug() << "Locale:" << locale.name();

	if (locale.name() != "pl_PL") {
		// TODO: Load translations according to locale variable.  En_US should
		// stay as fallback.  This can't be reliably implemented before adding
		// more supported languages though.
		appTranslator.load("fotorelacjonusz_en_US", ":/i18n");
	}

	sysTranslator.load(locale, "qtbase", "_", "/usr/share/qt5/translations") or
			sysTranslator.load(locale, "qtbase", "_", ":/i18n");

	installTranslator(&sysTranslator);
	installTranslator(&appTranslator);
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
