#include "application.h"
#include "exception.h"

Application::Application(int &argc, char **argv):
	QApplication(argc, argv)
{
}

bool Application::notify(QObject *object, QEvent *event)
{
	try
	{
		return QApplication::notify(object, event);
	}
	catch (const Exception &e)
	{
		e.showMessage(0);
	}
	return false;
}
