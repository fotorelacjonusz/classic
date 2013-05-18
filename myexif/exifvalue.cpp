#include "exifvalue.h"

#include <QVariant>

ExifValueBase::ExifValueBase(quint32 type):
	type(type)
{
}

ExifValueBase::~ExifValueBase()
{
}

template <class T, class TInner>
class ExifValueTemplate : public ExifValueBase
{
public:
	typedef T Type;
	typedef TInner TypeInner;
	
	ExifValueTemplate(const T &values, quint32 type):
		ExifValueBase(type),
		values(values)
	{
	}
	
	virtual quint32 count() const
	{
		return type == ExifValue::Ascii ? values.count() + 1 : values.count();
	}
	
private:
	T values;
	
	friend class ExifValue;
};

typedef ExifValueTemplate<QVector<quint8>,			quint8>			ExifValueByte;
typedef ExifValueTemplate<QVector<quint16>,			quint16>		ExifValueShort;
typedef ExifValueTemplate<QVector<quint32>,			quint32>		ExifValueLong;
typedef ExifValueTemplate<QVector<qint32>,			qint32>			ExifValueSLong;
typedef ExifValueTemplate<QString,					void>			ExifValueAscii;
typedef ExifValueTemplate<QByteArray,				void>			ExifValueUndefined;
typedef ExifValueTemplate<QVector<ExifURational>,	ExifURational>	ExifValueRational;
typedef ExifValueTemplate<QVector<ExifSRational>,	ExifSRational>	ExifValueSRational;

ExifValue::ExifValue()
{
}

ExifValue::ExifValue(QDataStream &stream)
{
	quint16 type;
	stream >> type;
	switch (type)
	{
		case Byte:		readValue<ExifValueByte>		(stream, type); break;
		case Ascii:		readString						(stream, type); break;
		case Short:		readValue<ExifValueShort>		(stream, type); break;
		case Long:		readValue<ExifValueLong>		(stream, type); break;
		case Rational:	readValue<ExifValueRational>	(stream, type); break;
		case Undefined:	readUndefined					(stream, type); break;
        case SLong:		readValue<ExifValueSLong>		(stream, type); break;
        case SRational:	readValue<ExifValueSRational>	(stream, type); break;
	}
}

ExifValue::~ExifValue()
{
}

bool ExifValue::isNull() const
{
	return value.isNull();
}

int ExifValue::type() const
{
	return value ? value->type : 0;
}

int ExifValue::count() const
{
	return value ? value->count() : 0;
}

void ExifValue::write(QDataStream &stream, QDataStream &valueStream) const
{
	stream << value->type;
	switch (value->type)
	{
		case Byte:		writeValue<ExifValueByte>		(stream, valueStream); break;
		case Ascii:		writeString						(stream, valueStream); break;
		case Short:		writeValue<ExifValueShort>		(stream, valueStream); break;
		case Long:		writeValue<ExifValueLong>		(stream, valueStream); break;
		case Rational:	writeValue<ExifValueRational>	(stream, valueStream); break;
		case Undefined:	writeUndefined					(stream, valueStream); break;
        case SLong:		writeValue<ExifValueSLong>		(stream, valueStream); break;
        case SRational:	writeValue<ExifValueSRational>	(stream, valueStream); break;
	}
}

#define FROM_SINGLE(ENUM) this->value = QSharedPointer<ExifValueBase>(new ExifValue##ENUM(ExifValue##ENUM::Type() << value, ENUM));
#define FROM_VECTOR(ENUM) this->value = QSharedPointer<ExifValueBase>(new ExifValue##ENUM(value, ENUM));

ExifValue::ExifValue(quint8 value)
{
	FROM_SINGLE(Byte);
}
ExifValue::ExifValue(const QVector<quint8> &value)
{
	FROM_VECTOR(Byte);
}
ExifValue::ExifValue(const QString &value)
{
	FROM_VECTOR(Ascii);
}
ExifValue::ExifValue(const char *str)
{
	QString value(str);
	FROM_VECTOR(Ascii);
}
ExifValue::ExifValue(quint16 value)
{
	FROM_SINGLE(Short);
}
ExifValue::ExifValue(const QVector<quint16> &value)
{
	FROM_VECTOR(Short);
}
ExifValue::ExifValue(quint32 value)
{
	FROM_SINGLE(Long);
}
ExifValue::ExifValue(const QVector<quint32> &value)
{
	FROM_VECTOR(Long);
}
ExifValue::ExifValue(const ExifURational &value)
{
	FROM_SINGLE(Rational);
}
ExifValue::ExifValue(const QVector<ExifURational> &value)
{
	FROM_VECTOR(Rational);
}
ExifValue::ExifValue(const QByteArray &value)
{
	FROM_VECTOR(Undefined);
}
ExifValue::ExifValue(qint32 value)
{
	FROM_SINGLE(SLong);
}
ExifValue::ExifValue(const QVector<qint32> &value)
{
	FROM_VECTOR(SLong);
}
ExifValue::ExifValue(const ExifSRational &value)
{
	FROM_SINGLE(SRational);
}
ExifValue::ExifValue(const QVector<ExifSRational> &value)
{
	FROM_VECTOR(SRational);
}
ExifValue::ExifValue(const QDateTime &dateTime)
{
	QString value = dateTime.toString("yyyy:MM:dd HH:mm:ss");
	FROM_VECTOR(Ascii);
}

#define TO_SINGLE(ENUM)					return value && value->type == ENUM && value->count() == 1 ? static_cast<const ExifValue##ENUM *>(value.data())->values.at(0) : 0;
#define TO_SINGLE_DEF(ENUM, DEFAULT)	return value && value->type == ENUM && value->count() == 1 ? static_cast<const ExifValue##ENUM *>(value.data())->values.at(0) : DEFAULT();
#define TO_VECTOR(ENUM)					return value && value->type == ENUM ?                        static_cast<const ExifValue##ENUM *>(value.data())->values : ExifValue##ENUM::Type();
#define TO_VECTOR_DEF(ENUM, DEFAULT)	return value && value->type == ENUM ?                        static_cast<const ExifValue##ENUM *>(value.data())->values : DEFAULT();

quint8 ExifValue::toByte() const
{
	TO_SINGLE(Byte);
}
QVector<quint8> ExifValue::toByteVector() const
{
	TO_VECTOR(Byte);
}
QString ExifValue::toString() const
{
	TO_VECTOR_DEF(Ascii, QString);
}
quint16 ExifValue::toShort() const
{
	TO_SINGLE(Short);
}
QVector<quint16> ExifValue::toShortVector() const
{
	TO_VECTOR(Short);
}
quint32 ExifValue::toLong() const
{
	TO_SINGLE(Long);
}
QVector<quint32> ExifValue::toLongVector() const
{
	TO_VECTOR(Long);
}
ExifURational ExifValue::toRational() const
{
	TO_SINGLE_DEF(Rational, ExifURational);
}
QVector<ExifURational> ExifValue::toRationalVector() const
{
	TO_VECTOR(Rational);
}
QByteArray ExifValue::toByteArray() const
{
	TO_VECTOR_DEF(Undefined, QByteArray);
}
qint32 ExifValue::toSignedLong() const
{
	TO_SINGLE(SLong);
}
QVector<qint32> ExifValue::toSignedLongVector() const
{
	TO_VECTOR(SLong);
}
ExifSRational ExifValue::toSignedRational() const
{
	TO_SINGLE_DEF(SRational, ExifSRational);
}
QVector<ExifSRational> ExifValue::toSignedRationalVector() const
{
	TO_VECTOR(SRational);
}
QDateTime ExifValue::toDateTime() const
{
	return QDateTime::fromString(toString(), "yyyy:MM:dd HH:mm:ss");
}

#pragma GCC diagnostic ignored "-Wtype-limits"

template <class T>
QVector<T> ExifValue::readVector(QDataStream &stream)
{
	quint32 count;
	quint32 offset;
	stream >> count;
	
	QVector<T> vector(count);
	if (count * sizeof(T) <= 4)
	{
		vector.resize(4);
		for (uint i = 0; i < 4 / sizeof(T); ++i)
			stream >> vector[i];
		vector.resize(count);
	}
	else
	{
		stream >> offset;
		quint32 backup = stream.device()->pos();
		stream.device()->seek(offset);
		for (uint i = 0; i < count; ++i)
		{
			stream >> vector[i];
		}
		stream.device()->seek(backup);
	}
	return vector;
}

template <class T>
void ExifValue::readValue(QDataStream &stream, quint32 type)
{
	value = QSharedPointer<ExifValueBase>(new T(readVector<typename T::TypeInner>(stream), type));
}

void ExifValue::readString(QDataStream &stream, quint32 type)
{
	QVector<qint8> vector = readVector<qint8>(stream);
	QString string = QString::fromAscii((const char *)vector.constData(), vector.count() - 1);
	value = QSharedPointer<ExifValueBase>(new ExifValueAscii(string, type));
}

void ExifValue::readUndefined(QDataStream &stream, quint32 type)
{
	QVector<quint8> vector = readVector<quint8>(stream);
	QByteArray byteArray((const char *)vector.constData(), vector.count());
	value = QSharedPointer<ExifValueBase>(new ExifValueUndefined(byteArray, type));
}

template <class T>
void ExifValue::writeVector(QDataStream &stream, QDataStream &valueStream, QVector<T> vector) const
{
	quint32 count = vector.count();
	quint32 offset = valueStream.device()->pos();
	stream << count;
	
	if (count * sizeof(T) <= 4)
	{
		vector.resize(4);
		for (uint i = 0; i < 4 / sizeof(T); ++i)
			stream << vector[i];
	}
	else
	{
		stream << offset;
		for (uint i = 0; i < count; ++i)
			valueStream << vector[i];
	}
}

template <class T>
void ExifValue::writeValue(QDataStream &stream, QDataStream &valueStream) const
{
	const QVector<typename T::TypeInner> &vector = static_cast<const T *>(value.data())->values;
	writeVector(stream, valueStream, vector);
}

void ExifValue::writeString(QDataStream &stream, QDataStream &valueStream) const
{
	QByteArray data = toString().toAscii() + QByteArray("\0", 1);
	QVector<qint8> vector(data.size());
	memcpy(vector.data(), data.constData(), data.size());
	writeVector(stream, valueStream, vector);
}

void ExifValue::writeUndefined(QDataStream &stream, QDataStream &valueStream) const
{
	QByteArray data = toByteArray();
	QVector<quint8> vector(data.size());
	memcpy(vector.data(), data.constData(), data.size());
	writeVector(stream, valueStream, vector);
}


