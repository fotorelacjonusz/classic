#include "exception.h"
#include <QMessageBox>
#include <QDebug>

Exception::Exception(const char *file, int line, const char *func, QString what):
	file(QString(file).split('/').last()),
	func(func),
//	where(QString("%1:%2\n%3").arg(QString(file).split("/").last()).arg(line).arg(func)),
	what(what),
	line(line)
{
}

void Exception::showMessage(QWidget *parent) const
{
	QMessageBox::critical(parent, tr("Błąd"), tr("%1:%2\n%3\n\nTreść błędu:\n%4").arg(file).arg(line).arg(func).arg(what));
}

QString Exception::message() const
{
	return QString("%1:%2,\t%3:\t%4").arg(file).arg(line).arg(func).arg(what);
//	return what;
}
