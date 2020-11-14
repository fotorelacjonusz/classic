#pragma once

#include "exifrational.h"
#include "exception.h"
#include <QVector>
#include <QDateTime>
#include <QSharedPointer>

typedef quint8 exifbyte;
typedef quint16 exifshort;
typedef quint32 exiflong;
typedef qint32 exifslong;

class ExifValueBase
{
public:
	ExifValueBase(quint16 type = 0);
	virtual ~ExifValueBase();
	virtual exiflong count() const = 0;

protected:
	quint16 type;

	friend class ExifValue;
};

class ExifValue
{
public:
	enum Type
	{
		Byte		= 1,
		Ascii		= 2,
		Short		= 3,
		Long		= 4,
		Rational	= 5,
		Undefined	= 7,
		SLong		= 9,
		SRational	= 10
	};

	ExifValue();
	ExifValue(QDataStream &stream);
	~ExifValue();
	void write(QDataStream &stream, QDataStream &valueStream) const;


	bool operator ==(const ExifValue &other) const;

	bool isNull() const;
	int type() const;
	int count() const;

	ExifValue(exifbyte value);
	ExifValue(const QVector<exifbyte> &value);
	ExifValue(const QString &value);
	ExifValue(const char *str);
	ExifValue(exifshort value);
	ExifValue(const QVector<exifshort> &value);
	ExifValue(exiflong value);
	ExifValue(const QVector<exiflong> &value);
	ExifValue(const ExifURational &value);
	ExifValue(const QVector<ExifURational> &value);
	ExifValue(const QByteArray &value);
	ExifValue(exifslong value);
	ExifValue(const QVector<exifslong> &value);
	ExifValue(const ExifSRational &value);
	ExifValue(const QVector<ExifSRational> &value);
	ExifValue(const QDateTime &dateTime);

	exifbyte toByte() const;
	QVector<exifbyte> toByteVector() const;
	QString toString() const;
	exifshort toShort() const;
	QVector<exifshort> toShortVector() const;
	exiflong toLong() const;
	QVector<exiflong> toLongVector() const;
	ExifURational toRational() const;
	QVector<ExifURational> toRationalVector() const;
	QByteArray toByteArray() const;
	exifslong toSignedLong() const;
	QVector<exifslong> toSignedLongVector() const;
	ExifSRational toSignedRational() const;
	QVector<ExifSRational> toSignedRationalVector() const;
	QDateTime toDateTime() const;

private:
	template <class T>
	QVector<T> readVector(QDataStream &stream) noexcept(false);
	template <class T>
	void readValue(QDataStream &stream, quint16 type);
	void readString(QDataStream &stream, quint16 type);
	void readUndefined(QDataStream &stream, quint16 type);

	template <class T>
	void writeVector(QDataStream &stream, QDataStream &valueStream, QVector<T> vector) const;
	template <class T>
	void writeValue(QDataStream &stream, QDataStream &valueStream) const;
	void writeString(QDataStream &stream, QDataStream &valueStream) const;
	void writeUndefined(QDataStream &stream, QDataStream &valueStream) const;

	QSharedPointer<ExifValueBase> value;
};
