#ifndef EXIFEXCEPTION_H
#define EXIFEXCEPTION_H

#include <QString>

class ExifException
{
public:
	ExifException(const QString &reason);
	QString what() const;
	
private:
	const QString reason;
};

#endif // EXIFEXCEPTION_H
