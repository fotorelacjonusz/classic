#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QVersionNumber>

class Application : public QApplication
{
	Q_OBJECT
public:
	explicit Application(int &argc, char **argv);

	int showWindowAndExec();

	static QString applicationSettingsName();
	static QString applicationNameAndVersion();
	static QVersionNumber applicationVersionNumber();

	static void busy();
	static void idle();

protected:
	bool notify(QObject *object, QEvent *event);

};

#endif // APPLICATION_H
