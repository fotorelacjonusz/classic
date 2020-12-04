#pragma once

#include <QApplication>
#include <QTranslator>
#include <QVersionNumber>

class Application : public QApplication
{
	Q_OBJECT

	QTranslator appTranslator;
	QTranslator sysTranslator;

public:
	explicit Application(int &argc, char **argv);

	int showWindowAndExec();

	/**
	 * @brief Detects system locale, and enables appropriate translations.
	 */
	void setUpTranslations();

	static QString applicationSettingsName();
	static QString applicationNameAndVersion();
	static QVersionNumber applicationVersionNumber();

	static void busy();
	static void idle();

protected:
	bool notify(QObject *object, QEvent *event);

};
