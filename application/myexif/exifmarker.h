#ifndef EXIFMARKER_H
#define EXIFMARKER_H

#include <QDataStream>
#include "exception.h"

class ExifMarker
{
public:
	enum MarkerNumber
	{
		APP0 = 0xe0,
		APP1 = 0xe1,
		SOI  = 0xd8,
		SOS  = 0xda
	};

	ExifMarker(QDataStream &stream) throw (Exception);
	ExifMarker(const ExifMarker &other, MarkerNumber number); // insert before
	bool isAPP0() const;
	bool isAPP1() const;
	bool isSOI() const;
	bool isSOS() const;

	QByteArray readData(const QByteArray &header = QByteArray()) const throw (Exception);
	void writeData(const QByteArray &data, const QByteArray &header = QByteArray());

private:
	QDataStream &stream;
	const int start;
	int end;
	quint8 ff, number;
	quint16 size;
};


#endif // EXIFMARKER_H
