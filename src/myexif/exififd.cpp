#include "exififd.h"
#include "messagehandler.h"
#include <QBuffer>
#include <QTextStream>

const QList<ExifIfd::EmbedOffset> ExifIfd::allPointers =
		QList<ExifIfd::EmbedOffset>() << ExifIfdPointer << GpsInfoIfdPointer << InteroperabilityIfdPointer;

ExifIfd::ExifIfd():
	nextIFD(0),
	nextIFDSought(false)
{
}

ExifIfd::ExifIfd(QDataStream &stream) throw (Exception):
	nextIFD(0),
	nextIFDSought(false)
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
			quint32 backupPos = stream.device()->pos();
			stream.device()->seek(value(pointerTag).toLong()) OR_THROW(SEEK_ERROR(value(pointerTag).toLong()));
			ifds[pointerTag] = ExifIfd(stream);
			if (ifds[pointerTag].hasNextIFDError())
				stream.device()->seek(backupPos);
		}
	if (contains(JpegInterchangeFormat) && contains(JpegInterchangeFormatLength))
	{
		quint32 thumbnailOffset = value(JpegInterchangeFormat).toLong();
		quint32 thumbnailSize = value(JpegInterchangeFormatLength).toLong();
		stream.device()->seek(thumbnailOffset) OR_THROW(SEEK_ERROR(thumbnailOffset));
		thumbnailImage.loadFromData(stream.device()->read(thumbnailSize)) OR_THROW("Unable to load thumbnail");
	}
	Suppress(), nextIFDSought = stream.device()->seek(nextIFD); // OR_THROW(SEEK_ERROR(nextIFD));
	if (hasNextIFDError())
		QTextStream(stderr) << "Embedded exif ifd malformed, invalid nextIFD: " << nextIFD << "\n";
}

void ExifIfd::write(QDataStream &stream, QByteArray &data, bool hasNext) const
{
	stream << quint16(size());

	QDataStream valueStream(&data, QIODevice::WriteOnly);
	valueStream.setByteOrder(stream.byteOrder());
	valueStream.device()->seek(stream.device()->pos() + 12 * size() + sizeof(nextIFD));
	exiflong imageLength = 0;

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
			ExifValue(exiflong(valueStream.device()->pos())).write(stream, valueStream);
			exiflong before = valueStream.device()->pos();
			thumbnailImage.save(valueStream.device(), "JPG");
			imageLength = valueStream.device()->pos() - before;
		}
		else if (tag == JpegInterchangeFormatLength)
			ExifValue(exiflong(imageLength)).write(stream, valueStream);
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
	if (image.isNull())
	{
		remove(JpegInterchangeFormat);
		remove(JpegInterchangeFormatLength);
		return;
	}
	insert(JpegInterchangeFormat, ExifValue(exiflong(0)));
	insert(JpegInterchangeFormatLength, ExifValue(exiflong(0)));

//	if (image.width() > 160 || image.height() > 120)
//		image = image.scaled(QSize(160, 120), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	thumbnailImage = image;
}

bool ExifIfd::hasNextIFDError() const
{
	return !nextIFDSought;
}
