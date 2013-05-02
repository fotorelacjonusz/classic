#include "gpsdata.h"
#include "settingsdialog.h"

#include <QFile>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPainter>
#include <QEventLoop>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QApplication>
#include <QDesktopServices>
#include <qmath.h>

#ifdef Q_WS_WIN
QString GpsData::binName = "exif.exe";
//#define binName "exif.exe"
#else
QString GpsData::binName = "./exif";
#endif

QHash<int, QImage> GpsData::maskCache;
QHash<GpsData *, QPointF> GpsData::allCoords;

GpsData::GpsData(const int *const number, QString filePath):
	number(number),
	hasPosition(false),
	hasDirection(false),
	inCommonMode(false)
{
	init();
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

//	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));

	//	hasDirection = true;
	//	direction = 45;
}

GpsData::GpsData():
	number(0),
	hasPosition(false),
	hasDirection(false),
	inCommonMode(true)
{
	init();
}

GpsData::GpsData(const int *const number, QDataStream &stream):
	number(number),
	inCommonMode(false)
{
	init();
	stream >> hasPosition >> hasDirection >> latitude >> longitude >> direction;
	if (hasPosition)
		allCoords[this] = QPointF(longitude, latitude);
//	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

void GpsData::init()
{
//	cache.setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
//	manager.setCache(&cache);
	connect(&googleDownloader, SIGNAL(mapDownloaded(QImage)), this, SLOT(mapDownloaded(QImage)));
	connect(&tilesDownloader, SIGNAL(mapDownloaded(QImage)), this, SLOT(mapDownloaded(QImage)));
}

GpsData::~GpsData()
{
	allCoords.remove(this);
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

void GpsData::downloadMap()
{
	if (inCommonMode)
	{
		if (allCoords.isEmpty())
			emit mapReady(QPixmap());
		else
		{
//			AbstractMapDownloader::Points points;
//			for (QHash<GpsData *, QPointF>::Iterator i = allCoords.begin(); i != allCoords.end() )
			(SETTINGS->commonMapType < SETTINGS->OSMFirst ? (AbstractMapDownloader *)&googleDownloader : (AbstractMapDownloader *)&tilesDownloader)
				->downloadMap(allCoords);
		}
	}
	else if (hasPosition)
	{
		if (SETTINGS->useOverlays)
		{
			QPixmap overlayMap = SETTINGS->overlayMakeMap(longitude, latitude);
			if (!overlayMap.isNull())
			{
				mapDownloaded(overlayMap.toImage());
				return;
			}
		}
		(SETTINGS->imageMapType < SETTINGS->OSMFirst ? (AbstractMapDownloader *)&googleDownloader : (AbstractMapDownloader *)&tilesDownloader)
			->downloadMap(QPointF(longitude, latitude));
	}
}

void GpsData::mapDownloaded(QImage map)
{
	if (map.isNull())
		return;
	if (!inCommonMode)
		processMap(map);
	
	emit mapReady(QPixmap::fromImage(map));
}

void GpsData::processMap(QImage &map)
{
	if (map.format() != QImage::Format_ARGB32_Premultiplied)
		map = map.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	
	QColor color = SETTINGS->imageMapColor;
//	QPoint center = QPoint(map.width() / 2.0, map.height() / 2.0);
	QPainter painter(&map);
	painter.setPen(QPen(color, 2));
	color.setAlpha(50);
	painter.setBrush(QBrush(color));
	painter.setRenderHint(QPainter::Antialiasing);

	if (hasDirection)
		painter.drawPie(map.rect().adjusted(-200, -200, 200, 200), -16 * (direction - 90 - 30), -16 * 60);
	else
		painter.drawEllipse(map.rect().center(), 7, 7);
	if (SETTINGS->imageMapCircle)
	{
		painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
		painter.drawImage(QPoint(0, 0), mask(map.size()));
	}
//	painter.end();
}

QImage GpsData::mask(QSize size)
{
	Q_ASSERT(size.width() == size.height());
	int sizeKey = size.width();

	if (maskCache.contains(sizeKey))
		return maskCache[sizeKey];

	QImage mask(size, QImage::Format_ARGB32);
	mask.fill(Qt::transparent);
	QPainter painter(&mask);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(QBrush(Qt::black));
	painter.drawPie(mask.rect(), 0, 16 * 360);
//	painter.end();

	return maskCache[sizeKey] = mask;
//	return mask;
}
