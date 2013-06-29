#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class Application : public QApplication
{
	Q_OBJECT
public:
	explicit Application(int &argc, char **argv);
	static QString applicationSettingsName();
	static QString applicationNameAndVersion();
	
	static void busy();
	static void idle();
	
protected:
	bool notify(QObject *object, QEvent *event);
	
};

class Version
{
public:
	Version(QString version = QApplication::applicationVersion());
	bool isPhrCompatible(const Version &other);
	
	const int major;
	const int format;
	const int minor;
};

#endif // APPLICATION_H
