#ifndef EXIFEXCEPTION_H
#define EXIFEXCEPTION_H

#include <QString>

#define SEEK_ERROR(x) ExifException(QString("Wrong offset: %1").arg(x))

/*
class ExifException
{
public:
	ExifException(const QString &reason);
	QString what() const;
	
private:
	const QString reason;
};
*/

#endif // EXIFEXCEPTION_H
