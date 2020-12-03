#pragma once

#include <QApplication>
#include <QVersionNumber>

class Application : public QApplication
{
	Q_OBJECT
public:
	explicit Application(int &argc, char **argv);

	int showWindowAndExec();
	void setUpTranslations();

	static QString applicationSettingsName();
	static QString applicationNameAndVersion();
	static QVersionNumber applicationVersionNumber();

	static void busy();
	static void idle();

protected:
	bool notify(QObject *object, QEvent *event);

};
