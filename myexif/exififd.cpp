#include "exififd.h"
#include "exifexception.h"
#include <QBuffer>

const QList<ExifIfd::EmbedOffset> ExifIfd::allPointers = 
		QList<ExifIfd::EmbedOffset>() << ExifIfdPointer << GpsInfoIfdPointer << InteroperabilityIfdPointer;

ExifIfd::ExifIfd():
	nextIFD(0)
{
}

ExifIfd::ExifIfd(QDataStream &stream):
	nextIFD(0)
{
	quint16 size;
	stream >> size;
	
	quint16 tag;
	for (int i = 0; i < size; ++i)
	{
		stream >> tag;
		insert(tag, ExifValue(stream));
	}
	stream >> nextIFD;
	
	foreach (EmbedOffset pointerTag, allPointers)
		if (contains(pointerTag))
		{
			stream.device()->seek(value(pointerTag).toLong());
			ifds[pointerTag] = ExifIfd(stream);
		}
	if (contains(JpegInterchangeFormat) && contains(JpegInterchangeFormatLength))
	{
		quint32 thumbnailOffset = value(JpegInterchangeFormat).toLong();
		quint32 thumbnailSize = value(JpegInterchangeFormatLength).toLong();
		stream.device()->seek(thumbnailOffset);
		if (!thumbnailImage.loadFromData(stream.device()->read(thumbnailSize)))
			throw ExifException("Unable to load thumbnail");
	}
	stream.device()->seek(nextIFD);
}

void ExifIfd::write(QDataStream &stream, QByteArray &data, bool hasNext) const
{
	stream << quint16(size());
	
	QDataStream valueStream(&data, QIODevice::WriteOnly);
	valueStream.setByteOrder(stream.byteOrder());
	valueStream.device()->seek(stream.device()->pos() + 12 * size() + sizeof(nextIFD));
	quint32 imageLength = 0;
	
	for (ValueMap::ConstIterator i = constBegin(); i != constEnd(); ++i)
	{
		quint16 tag = i.key();
		stream << tag;
		if (allPointers.contains(EmbedOffset(tag)))
		{
			ExifValue(quint32(valueStream.device()->pos())).write(stream, valueStream);
			ifds.value(EmbedOffset(tag)).write(valueStream, data, false);
		}
		else if (tag == JpegInterchangeFormat)
		{
			ExifValue(quint32(valueStream.device()->pos())).write(stream, valueStream);
			quint32 before = valueStream.device()->pos();
			thumbnailImage.save(valueStream.device(), "JPG");
			imageLength = valueStream.device()->pos() - before;
		}
		else if (tag == JpegInterchangeFormatLength)
			ExifValue(quint32(imageLength)).write(stream, valueStream);
		else
			i.value().write(stream, valueStream);	
	}

	quint32 nextIFD = hasNext ? valueStream.device()->pos() : 0;
	stream << nextIFD;
	stream.device()->seek(valueStream.device()->pos());
}

bool ExifIfd::hasNext() const
{
	return nextIFD;
}

const ExifIfd ExifIfd::embededIfd(EmbedOffset tag) const
{
	return ifds[tag];
}

ExifIfd &ExifIfd::embededIfd(EmbedOffset tag)
{
	if (!ifds.contains(tag))
		insert(tag, ExifValue(quint32(0)));
	return ifds[tag];
}

QImage ExifIfd::thumbnail() const
{
	return thumbnailImage;
}

void ExifIfd::setThumbnail(QImage image)
{
	if (image.width() > 160 || image.height() > 120)
		image = image.scaled(QSize(160, 120), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	if (thumbnailImage.isNull())
		insert(JpegInterchangeFormat, ExifValue(quint32(0)));
	thumbnailImage = image;
}
