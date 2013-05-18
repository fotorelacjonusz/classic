#ifndef EXIFIFD_H
#define EXIFIFD_H

#include "exifvalue.h"

#include <QMap>
#include <QImage>

typedef QMap<int, ExifValue> ValueMap;
class ExifIfd : public ValueMap
{
public:
	enum EmbedOffset
    {
        ExifIfdPointer              = 0x8769,
        GpsInfoIfdPointer           = 0x8825,
        InteroperabilityIfdPointer  = 0xA005,
        JpegInterchangeFormat       = 0x0201,
        JpegInterchangeFormatLength = 0x0202
    };
	
	ExifIfd();
	ExifIfd(QDataStream &stream);
	void write(QDataStream &stream, QByteArray &data, bool hasNext) const;
	
	bool hasNext() const;
	
	const ExifIfd embededIfd(EmbedOffset tag) const;
	ExifIfd &embededIfd(EmbedOffset tag);
	QImage thumbnail() const;
	void setThumbnail(QImage image);
	
private:
	QMap<EmbedOffset, ExifIfd> ifds;
	quint32 nextIFD;
	QImage thumbnailImage;
	
	static const QList<EmbedOffset> allPointers;
};

#endif // EXIFIFD_H



