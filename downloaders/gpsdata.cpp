#include "gpsdata.h"
#include "settings/settingsdialog.h"
#include "geomap.h"
#include "googlemapsdownloader.h"
#include "tilesdownloader.h"

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

#ifdef Q_WS_WIN
QString GpsData::binName = "exif.exe";
//#define binName "exif.exe"
#else
QString GpsData::binName = "./exif";
#endif

GpsData::Points GpsData::allCoords;

GpsData::GpsData(const int *const number, QString filePath):
	number(number),
	hasPosition(false),
	hasDirection(false),
	isCommon(false)
{
	initDownloaders();
	if (!QFile::exists(filePath))
		return;

#ifndef Q_WS_WIN
	if (!QFile::exists(binName))
		binName = "/usr/bin/fotorelacjonusz-exif";
#endif

	if (!QFile::exists(binName))
	{
		QMessageBox::critical(0, tr("Błąd"), tr("Brak pliku %1. Ponowna instalacja aplikacji może rozwiązać problem.").arg(binName));
		qApp->quit();
		return;
	}

	QProcess process;
	QStringList arguments;
	arguments << filePath;
	process.start(binName, arguments);
	process.waitForFinished(5000);
	QString output = process.readAll();

	if (process.exitCode() != 0 || process.exitStatus() != QProcess::NormalExit)
	{
		if (!output.isEmpty())
			QMessageBox::critical(0, tr("Błąd"), output);
		return;
	}

	QStringList list = output.split(",");
	if (list.size() >= 2)
	{
		hasPosition = true;
		latitude = list.takeFirst().toDouble();
		longitude = list.takeFirst().toDouble();
		allCoords[this] = QPointF(longitude, latitude);

		if (list.size() >= 1)
		{
			hasDirection = true;
			direction = list.takeFirst().toDouble();
		}
	}

	//	hasDirection = true;
	//	direction = 45;
}

GpsData::GpsData():
	number(0),
	hasPosition(false),
	hasDirection(false),
	isCommon(true)
{
	initDownloaders();
}

GpsData::GpsData(const int *const number, QDataStream &stream):
	number(number),
	isCommon(false)
{
	initDownloaders();
	stream >> hasPosition >> hasDirection >> latitude >> longitude >> direction;
	if (hasPosition)
		allCoords[this] = QPointF(longitude, latitude);
}

void GpsData::initDownloaders()
{
	googleDownloader = new GoogleMapsDownloader();
	tilesDownloader = new TilesDownloader();
}

GpsData::~GpsData()
{
	allCoords.remove(this);
	delete googleDownloader;
	delete tilesDownloader;
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
		if (allCoords.isEmpty())
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
		GeoMap *map = new GeoMap(QPointF(longitude, latitude), hasDirection, direction, mapSize);
		connect(map, SIGNAL(ready(QImage)), this, SIGNAL(mapReady(QImage)));
		if (!(SETTINGS->useOverlays && SETTINGS->makeMap(map)))
			(SETTINGS->imageMapType < SettingsDialog::OSMFirst ? googleDownloader : tilesDownloader)->makeMap(map);
	}
}
