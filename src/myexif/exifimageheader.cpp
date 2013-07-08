#include "exifimageheader.h"
#include "exififd.h"
#include "exifmarker.h"

#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QBuffer>
#include <QIODevice>
#include <qmath.h>

const QByteArray ExifImageHeader::exifHeader("Exif\0\0", 6);

ExifImageHeader::ExifImageHeader()
{
}

ExifImageHeader::ExifImageHeader(const QString &filePath)
{
	loadFromJpeg(filePath);
}

ExifImageHeader::~ExifImageHeader()
{
}

bool ExifImageHeader::loadFromJpeg(const QString &filePath)
{
	QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
        return loadFromJpeg(&file);
    else
        return false;
}

bool ExifImageHeader::loadFromJpeg(QIODevice *device)
{
	if (device->isSequential())
		return false;
	try
	{
		QDataStream stream(device);
		loadFromJpeg(stream);
	}
	catch (Exception &e)
	{
		QTextStream(stderr) << "Loading exif data failed:\n" << e.message() << "\n";
		return false;
	}
	return true;
}

bool ExifImageHeader::saveToJpeg(const QString &filePath) const
{
	QFile file(filePath);
    if (file.open(QIODevice::ReadWrite))
        return saveToJpeg(&file);
    else
        return false;
}

bool ExifImageHeader::saveToJpeg(QIODevice *device) const
{
	if (device->isSequential())
		return false;
	try
	{
		QDataStream stream(device);
		saveToJpeg(stream);
	}
	catch (Exception &e)
	{
		QTextStream(stderr) << "Saving exif data failed:\n" << e.message() << "\n";
		return false;
	}
	return true;
}

QList<ExifImageHeader::ImageTag> ExifImageHeader::imageTags() const
{
	QList<ExifImageHeader::ImageTag> tags;
	foreach (int key, ifds[0].keys())
		tags << (ExifImageHeader::ImageTag)key;
	return tags;
}

QList<ExifImageHeader::ExtendedTag> ExifImageHeader::extendedTags() const
{
	QList<ExifImageHeader::ExtendedTag> tags;
	foreach (int key, exifIFD().keys())
		tags << (ExifImageHeader::ExtendedTag)key;
	return tags;
}

QList<ExifImageHeader::GpsTag> ExifImageHeader::gpsTags() const
{
	QList<ExifImageHeader::GpsTag> tags;
	foreach (int key, gpsIFD().keys())
		tags << (ExifImageHeader::GpsTag)key;
	return tags;
}

bool ExifImageHeader::contains(ExifImageHeader::ImageTag tag) const
{
	return ifds[0].contains(tag);
}

bool ExifImageHeader::contains(ExifImageHeader::ExtendedTag tag) const
{
	return exifIFD().contains(tag);
}

bool ExifImageHeader::contains(ExifImageHeader::GpsTag tag) const
{
	return gpsIFD().contains(tag);
}

void ExifImageHeader::remove(ExifImageHeader::ImageTag tag)
{
	ifds[0].remove(tag);
}

void ExifImageHeader::remove(ExifImageHeader::ExtendedTag tag)
{
	exifIFD().remove(tag);
}

void ExifImageHeader::remove(ExifImageHeader::GpsTag tag)
{
	gpsIFD().remove(tag);
}

ExifValue ExifImageHeader::value(ExifImageHeader::ImageTag tag) const
{
	return ifds[0].value(tag);
}

ExifValue ExifImageHeader::value(ExifImageHeader::ExtendedTag tag) const
{
	return exifIFD().value(tag);
}

ExifValue ExifImageHeader::value(ExifImageHeader::GpsTag tag) const
{
	return gpsIFD().value(tag);
}

void ExifImageHeader::setValue(ExifImageHeader::ImageTag tag, const ExifValue &value)
{
	ifds[0][tag] = value;
}

void ExifImageHeader::setValue(ExifImageHeader::ExtendedTag tag, const ExifValue &value)
{
	exifIFD()[tag] = value;
}

void ExifImageHeader::setValue(ExifImageHeader::GpsTag tag, const ExifValue &value)
{
	gpsIFD()[tag] = value;
}

QImage ExifImageHeader::thumbnail() const
{
	return ifds[1].thumbnail();
}

void ExifImageHeader::setThumbnail(const QImage &thumbnail)
{
	ifds[1].setThumbnail(thumbnail);
}

ExifImageHeader::OrientationTag ExifImageHeader::orientation() const
{
	if (!contains(Orientation))
		return Horizontal;
	
	OrientationTag orientation = OrientationTag(value(Orientation).toShort());
	if (Horizontal <= orientation && orientation <= RotatedLeft)
		return orientation;
	
	qDebug() << "Exif header Orientation has invalid value:" << orientation << "Defaulting to Horizontal";
	return Horizontal;
}

void ExifImageHeader::setOrientation(ExifImageHeader::OrientationTag orientation)
{
	setValue(Orientation, exifshort(orientation));
}

QMatrix ExifImageHeader::reverseMatrixForOrientation() const
{
	static const int matrices[8][4] = 
	{
		{ 1, 0, 0, 1}, // identity
		{-1, 0, 0, 1}, // horizontal mirror
		{-1, 0, 0,-1}, // rotation 180
		{ 1, 0, 0,-1}, // vertical mirror 
		{ 0, 1, 1, 0}, // diagonal mirror 
		{ 0,-1, 1, 0}, // rotation 90 left
		{ 0,-1,-1, 0}, // antidiagonal mirror
		{ 0, 1,-1, 0}  // rotation 90 right
	};
	
	const int *matrix = matrices[orientation() - 1];
	return QMatrix(matrix[0], matrix[1], matrix[2], matrix[3], 0, 0);
}

QPointF ExifImageHeader::gpsPosition() const
{
	if (contains(GpsLatitude)  && contains(GpsLatitudeRef) && 
		contains(GpsLongitude) && contains(GpsLongitudeRef))
		return QPointF(dmsToReal(value(GpsLongitude), value(GpsLongitudeRef)),
					   dmsToReal(value(GpsLatitude),  value(GpsLatitudeRef)));
	return QPointF();	
}

void ExifImageHeader::setGpsPosition(QPointF position)
{
	if (position.isNull())
	{
		remove(GpsLongitude);
		remove(GpsLongitudeRef);
		remove(GpsLatitude);
		remove(GpsLatitudeRef);
	}
	else
	{
		setValue(GpsLongitude,    realToDms(position.x()));
		setValue(GpsLongitudeRef, position.x() < 0 ? 'W' : 'E');
		setValue(GpsLatitude,     realToDms(position.y()));
		setValue(GpsLatitudeRef,  position.y() < 0 ? 'S' : 'N');
	}
}

qreal ExifImageHeader::dmsToReal(const ExifValue &dms, const ExifValue &ref)
{
	char c = ref.toString().toAscii()[0];
	const QVector<ExifURational> vector = dms.toRationalVector();
	return (vector[0].toReal() + vector[1].toReal() / 60 + vector[2].toReal() / 3600) * ((c == 'S' || c == 'W') ? -1 : 1);
}

ExifValue ExifImageHeader::realToDms(qreal real)
{
	real = qAbs(real);
	QVector<ExifURational> vector(3);
	vector[0] = ExifURational(qFloor(real), 1);
	vector[1] = ExifURational(qFloor((real -= vector[0].first) *= 60), 1);
	vector[2] = ExifURational(qFloor((real -= vector[1].first) *= 60 * 1000), 1000);
	return vector;
}

//ExifValue ExifImageHeader::realtoRef(qreal real, bool lon)
//{
//	return lon ? (real < 0 ? 'W' : 'E') : (real < 0 ? 'S' : 'N');
//}

const ExifIfd ExifImageHeader::exifIFD() const
{
	return ifds[0].embededIfd(ExifIfd::ExifIfdPointer);
}

const ExifIfd ExifImageHeader::gpsIFD() const
{
	return ifds[0].embededIfd(ExifIfd::GpsInfoIfdPointer);
}

ExifIfd &ExifImageHeader::exifIFD()
{
	return ifds[0].embededIfd(ExifIfd::ExifIfdPointer);
}

ExifIfd &ExifImageHeader::gpsIFD()
{
	return ifds[0].embededIfd(ExifIfd::GpsInfoIfdPointer);
}

void ExifImageHeader::loadFromJpeg(QDataStream &fileStream) throw (Exception)
{
	fileStream.setByteOrder(QDataStream::BigEndian);
	ExifMarker(fileStream).isSOI() OR_THROW("First marker is not SOI");
	
	while (!fileStream.atEnd())
	{
		ExifMarker marker(fileStream);
		if (marker.isAPP1())
		{
			QByteArray data = marker.readData(exifHeader);
			QDataStream stream(data);
			
			QByteArray align = stream.device()->read(2);
			if (align == "II")
				byteOrder = QDataStream::LittleEndian;
			else if (align == "MM")
				byteOrder = QDataStream::BigEndian;
			else
				THROW("Unknown align");
			
			stream.setByteOrder(byteOrder);
			quint16 id;
		    quint32 offset;
			stream >> id >> offset;
			id == 0x002a OR_THROW("Bad TIFF header");
			stream.device()->seek(offset) OR_THROW(SEEK_ERROR(offset));
			do
				ifds.insert(ifds.size(), ExifIfd(stream));
			while (ifds[ifds.size() - 1].hasNext());
			break;
		}
		else if (marker.isSOS())
			return;
	}
	!fileStream.atEnd() OR_THROW("APP1 marker not found");
}

void ExifImageHeader::saveToJpeg(QDataStream &fileStream) const throw (Exception)
{
	fileStream.setByteOrder(QDataStream::BigEndian);
	ExifMarker(fileStream).isSOI() OR_THROW("First marker is not SOI");
	
	ExifMarker m1(fileStream);
	ExifMarker m2(fileStream);
	
	if (m1.isAPP1())      // SOI -> APP1
		saveToJpeg(m1);
	else if (m2.isAPP1()) // SOI -> APP0? -> APP1
		saveToJpeg(m2);
	else if (m1.isAPP0()) // SOI -> APP0 -> ???
		saveToJpeg(ExifMarker(m2, ExifMarker::APP1));
	else                  // SOI -> ???
		saveToJpeg(ExifMarker(m1, ExifMarker::APP1));
}

void ExifImageHeader::saveToJpeg(ExifMarker app1) const throw (Exception)
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);
	stream.setByteOrder(byteOrder);
	stream.device()->write(byteOrder == QDataStream::LittleEndian ? "II" : "MM", 2);
	stream << quint16(0x002a) << quint32(8);

	for (ExifIfdMap::ConstIterator i = ifds.constBegin(); i != ifds.constEnd(); ++i)
		i.value().write(stream, data, i != ifds.constEnd() - 1);

	app1.writeData(data, exifHeader);
	return;
}
