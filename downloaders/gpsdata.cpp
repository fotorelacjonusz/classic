#include "gpsdata.h"
#include "settings/settingsdialog.h"
#include "geomap.h"
#include "googlemapsdownloader.h"
#include "tilesdownloader.h"
#include "myexif/exifimageheader.h"

#include <QFile>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QApplication>

#include <qmath.h>

GpsData::Points GpsData::allCoords;

GpsData::GpsData(QIODevice *image, const int *const number):
	number(number),
	hasPosition(false),
	hasDirection(false),
	isCommon(false),
	googleDownloader(new GoogleMapsDownloader()),
	tilesDownloader(new TilesDownloader()),
	exifHeader(new ExifImageHeader())
{
	if (!exifHeader->loadFromJpeg(image))
		return;
	
	if (exifHeader->contains(ExifImageHeader::GpsLatitude)  && exifHeader->contains(ExifImageHeader::GpsLatitudeRef) &&
		exifHeader->contains(ExifImageHeader::GpsLongitude) && exifHeader->contains(ExifImageHeader::GpsLongitudeRef))
	{
		hasPosition = true;
		latitude =  dmsToReal(exifHeader->value(ExifImageHeader::GpsLatitude),  exifHeader->value(ExifImageHeader::GpsLatitudeRef));
		longitude = dmsToReal(exifHeader->value(ExifImageHeader::GpsLongitude), exifHeader->value(ExifImageHeader::GpsLongitudeRef));
		allCoords[this] = QPointF(longitude, latitude);
	}
	if (exifHeader->contains(ExifImageHeader::GpsImageDirection) && exifHeader->contains(ExifImageHeader::GpsImageDirectionRef))
	{
		hasDirection = true;
		direction = exifHeader->value(ExifImageHeader::GpsImageDirection).toRational().toReal();
	}
	exifHeader->setValue(ExifImageHeader::Software, qApp->applicationName());

	SETTINGS->connectMany(this, SLOT(downloadMap()), &SETTINGS->addImageMap, &SETTINGS->imageMapType, &SETTINGS->imageMapColor,
						  &SETTINGS->imageMapOpacity, &SETTINGS->imageMapZoom, &SETTINGS->imageMapCircle, &SETTINGS->imageMapCorner,
						  &SETTINGS->imageMapMargin, &SETTINGS->imageMapSize, &SETTINGS->useOverlays);

	//	hasDirection = true;
	//	direction = 45;
}

GpsData::GpsData():
	number(0),
	hasPosition(false),
	hasDirection(false),
	isCommon(true),
	googleDownloader(new GoogleMapsDownloader()),
	tilesDownloader(new TilesDownloader()),
	exifHeader(new ExifImageHeader())
{
	SETTINGS->connectMany(this, SLOT(downloadMap()), &SETTINGS->numberImages, &SETTINGS->startingNumber, &SETTINGS->addCommonMap, 
						  &SETTINGS->commonMapType, &SETTINGS->imageMapColor, &SETTINGS->useOverlayCommonMap, &SETTINGS->imageLength,
						  &SETTINGS->imageMapCorner, &SETTINGS->imageMapSize);
}

GpsData::~GpsData()
{
	allCoords.remove(this);
	delete googleDownloader;
	delete tilesDownloader;
	delete exifHeader;
}

void GpsData::saveExif(QIODevice *device) const
{
	exifHeader->saveToJpeg(device);
}

QString GpsData::toString() const
{
	QString res;
	if (hasPosition)
		res += QString("%1, %2").arg(latitude).arg(longitude);
	if (hasDirection)
		res += QString(" @%1").arg(direction);
	return res;
}

qreal GpsData::dmsToReal(const ExifValue &dms, const ExifValue &ref)
{
	char c = ref.toString().toAscii()[0];
	const QVector<ExifURational> vector = dms.toRationalVector();
	return (vector[0].toReal() + vector[1].toReal() / 60 + vector[2].toReal() / 3600) * ((c == 'S' || c == 'W') ? -1 : 1);
}

void GpsData::serialize(QDataStream &stream) const
{
	stream << hasPosition << hasDirection << latitude << longitude << direction;
}

void GpsData::setPhotoSize(QSize size)
{
	photoSize = size;
	const QSize mapSizePrev = mapSize;
	updateMapSize();
	if ((mapSizePrev.isEmpty() || mapSize != mapSizePrev) && SETTINGS->addImageMap)
		downloadMap();
}

void GpsData::updateMapSize()
{
	switch (SETTINGS->imageMapCorner)
	{
		case SettingsDialog::ExpandLeft:
		case SettingsDialog::ExpandRight:
			mapSize = QSize(SETTINGS->imageMapSize, photoSize.height());
			break;
		case SettingsDialog::ExpandTop:
		case SettingsDialog::ExpandBottom:
			mapSize = QSize(photoSize.width(), SETTINGS->imageMapSize);
			break;
		default:
			mapSize = QSize(SETTINGS->imageMapSize, SETTINGS->imageMapSize);
	}	
}

void GpsData::downloadMap()
{
	updateMapSize();
	
	if (isCommon)
	{
		if (!SETTINGS->addCommonMap || allCoords.isEmpty())
			return mapReady(QImage());
		
		QList<QPointF> coords = allCoords.values(); // populate only
		for (QHash<GpsData *, QPointF>::Iterator i = allCoords.begin(); i != allCoords.end(); ++i)
			coords[*i.key()->number] = i.value(); // QList with indexes same as images' numbers
		
		GeoMap *map = new GeoMap(coords);
		connect(map, SIGNAL(ready(QImage)), this, SIGNAL(mapReady(QImage)));
		if (!(SETTINGS->useOverlayCommonMap && SETTINGS->makeMap(map)))
			(SETTINGS->commonMapType < SettingsDialog::OSMFirst ? googleDownloader : tilesDownloader)->makeMap(map);
	}
	else if (hasPosition)
	{
		if (!SETTINGS->addImageMap)
			return mapReady(QImage());
		
		GeoMap *map = new GeoMap(QPointF(longitude, latitude), hasDirection, direction, mapSize);
		connect(map, SIGNAL(ready(QImage)), this, SIGNAL(mapReady(QImage)));
		if (!(SETTINGS->useOverlays && SETTINGS->makeMap(map)))
			(SETTINGS->imageMapType < SettingsDialog::OSMFirst ? googleDownloader : tilesDownloader)->makeMap(map);
	}
}
