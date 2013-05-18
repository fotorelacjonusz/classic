#include "exifexception.h"

ExifException::ExifException(const QString &reason):
	reason(reason)
{
}

QString ExifException::what() const
{
	return reason;
}
