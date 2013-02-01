#include "exception.h"
#include <QMessageBox>

Exception::Exception(const char *file, int line, const char *func, QString what):
	where(QString("%1:%2\n%3").arg(QString(file).split("/").last()).arg(line).arg(func)),
	what(what)
{
}

void Exception::showMessage(QWidget *parent) const
{
	QMessageBox::critical(parent, QObject::tr("Błąd"), QObject::tr("%1\n\nMiejsce błędu:\n%2").arg(what).arg(where));
}
