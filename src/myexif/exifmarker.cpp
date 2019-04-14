#include "exifmarker.h"
#include <QBuffer>
#include <QFile>
#include <QDebug>

ExifMarker::ExifMarker(QDataStream &stream) throw (Exception):
	stream(stream),
	start(stream.device()->pos()),
	size(0)
{
	stream >> ff;
	ff == 0xff OR_THROW("Marker not beginning with FF");

	stream >> number;
//	qDebug() << QString::number(number, 16);
	if (isSOI())
		return;

	stream >> size;

	end	= stream.device()->pos() + size - sizeof(size);
	stream.device()->seek(end) OR_THROW(SEEK_ERROR(end));
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

bool ExifMarker::isSOS() const
{
	return number == SOS;
}

QByteArray ExifMarker::readData(const QByteArray &header) const throw (Exception)
{
	const int pos = stream.device()->pos();
	stream.device()->seek(start + sizeof(ff) + sizeof(number) + sizeof(size));

	header == stream.device()->read(header.size()) OR_THROW("Header not found");

	QByteArray data = stream.device()->read(size - sizeof(size));
	quint32(data.length()) == size - sizeof(size) OR_THROW("Marker early end");

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
