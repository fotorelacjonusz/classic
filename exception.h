#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <QString>

#define THROW(x) throw Exception(__FILE__, __LINE__, __PRETTY_FUNCTION__, x);

class QWidget;

class Exception
{
public:
	Exception(const char *file, int line, const char *func, QString what);
	void showMessage(QWidget *parent) const;
	void debug() const;

private:
	const QString where, what;
};

#endif // EXCEPTION_H
