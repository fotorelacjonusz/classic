#include "exifmarker.h"
#include "exifexception.h"
#include <QBuffer>
#include <QFile>
#include <QDebug>

ExifMarker::ExifMarker(QDataStream &stream):
	stream(stream),
	start(stream.device()->pos()),
	size(0)
{
	stream >> ff;
	if (ff != 0xff)
		throw ExifException("Marker not beginning with FF");
	
	stream >> number;
	if (isSOI())
		return;

	stream >> size;
	
	end	= stream.device()->pos() + size - sizeof(size);
	stream.device()->seek(end);
}

ExifMarker::ExifMarker(const ExifMarker &other, MarkerNumber number):
	stream(other.stream),
	start(other.start),
	end(other.start),
	ff(0xff),
	number(number),
	size(0)
{
}

bool ExifMarker::isAPP0() const
{
	return number == APP0;
}

bool ExifMarker::isAPP1() const
{
	return number == APP1;
}

bool ExifMarker::isSOI() const
{
	return number == SOI;
}

QByteArray ExifMarker::readData(const QByteArray &header) const
{
	const int pos = stream.device()->pos();
	stream.device()->seek(start + sizeof(ff) + sizeof(number) + sizeof(size));
	
	if (header != stream.device()->read(header.size()))
		throw ExifException("Header not found");
	
	QByteArray data = stream.device()->read(size - sizeof(size));
	if (quint32(data.length()) != size - sizeof(size))
		throw ExifException("Marker early end");
	
	stream.device()->seek(pos);
	return data;
}

void ExifMarker::writeData(const QByteArray &data, const QByteArray &header)
{
	stream.device()->seek(end);
	QByteArray remainder = stream.device()->readAll();
	
	stream.device()->seek(start);
	stream << ff << number << quint16(sizeof(size) + header.size() + data.size());
	stream.device()->write(header);
	stream.device()->write(data);
	end = stream.device()->pos();
	stream.device()->write(remainder);
	
	// truncate
	if (!stream.atEnd())
	{
		QBuffer *buffer = qobject_cast<QBuffer *>(stream.device());
		if (buffer)
		{
			buffer->buffer().resize(buffer->pos());
			return;
		}
		
		QFile *file = qobject_cast<QFile *>(stream.device());
		if (file)
		{
			file->resize(file->pos());
			return;
		}
	}
}
