#ifndef EXIFVALUE_H
#define EXIFVALUE_H

#include "exifrational.h"
#include <QVector>
#include <QDateTime>

class ExifValueBase
{
public:
	ExifValueBase(quint32 type = 0);
	virtual ~ExifValueBase();
	virtual quint32 count() const = 0;
	
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
	
	ExifValue(quint8 value);
	ExifValue(const QVector<quint8> &value);
	ExifValue(const QString &value);
	ExifValue(const char *str);
	ExifValue(quint16 value);
	ExifValue(const QVector<quint16> &value);
	ExifValue(quint32 value);
	ExifValue(const QVector<quint32> &value);
	ExifValue(const ExifURational &value);
	ExifValue(const QVector<ExifURational> &value);
	ExifValue(const QByteArray &value);
	ExifValue(qint32 value);
	ExifValue(const QVector<qint32> &value);
	ExifValue(const ExifSRational &value);
	ExifValue(const QVector<ExifSRational> &value);
	ExifValue(const QDateTime &dateTime);
	
	quint8 toByte() const;
	QVector<quint8> toByteVector() const;
	QString toString() const;
	quint16 toShort() const;
	QVector<quint16> toShortVector() const;
	quint32 toLong() const;
	QVector<quint32> toLongVector() const;
	ExifURational toRational() const;
	QVector<ExifURational> toRationalVector() const;
	QByteArray toByteArray() const;
	qint32 toSignedLong() const;
	QVector<qint32> toSignedLongVector() const;
	ExifSRational toSignedRational() const;
	QVector<ExifSRational> toSignedRationalVector() const;
	QDateTime toDateTime() const;
	
private:
	template <class T>
	QVector<T> readVector(QDataStream &stream);
	template <class T>
	void readValue(QDataStream &stream, quint32 type);
	void readString(QDataStream &stream, quint32 type);
	void readUndefined(QDataStream &stream, quint32 type);
	
	template <class T>
	void writeVector(QDataStream &stream, QDataStream &valueStream, QVector<T> vector) const;
	template <class T>
	void writeValue(QDataStream &stream, QDataStream &valueStream) const;
	void writeString(QDataStream &stream, QDataStream &valueStream) const;
	void writeUndefined(QDataStream &stream, QDataStream &valueStream) const;
	
	
	QSharedPointer<ExifValueBase> value;	
};

#endif // EXIFVALUE_H
