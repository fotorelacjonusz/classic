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
#include <qmath.h>

#ifdef Q_WS_WIN
QString GpsData::binName = "exif.exe";
//#define binName "exif.exe"
#else
QString GpsData::binName = "./exif";
#endif

QMap<int, QImage> GpsData::maskCache;
QMap<GpsData *, QPointF> GpsData::allCoords;

GpsData::GpsData(QString filePath):
	hasPosition(false),
	hasDirection(false),
	inCommonMode(false)
{
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

	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));

	//	hasDirection = true;
	//	direction = 45;
}

GpsData::GpsData():
	hasPosition(false),
	hasDirection(false),
	inCommonMode(true)
{
}

GpsData::GpsData(QDataStream &stream):
	inCommonMode(false)
{
	stream >> hasPosition >> hasDirection >> latitude >> longitude >> direction;
	if (hasPosition)
		allCoords[this] = QPointF(longitude, latitude);
	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
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
		return;
	if (!hasPosition)
		return;
	if (!SETTINGS->imageMapType.wasChanged() &&
		!SETTINGS->imageMapZoom.wasChanged() &&
		!SETTINGS->imageMapSize.wasChanged() &&
		!SETTINGS->useOverlays.wasChanged() &&
		!imageCache.isNull())
		return processMap(imageCache);

	if (SETTINGS->useOverlays)
	{
		QPixmap overlayMap = SETTINGS->overlayMakeMap(longitude, latitude);
		if (!overlayMap.isNull())
		{
			imageCache = overlayMap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
			processMap(imageCache);
			return;
		}
	}

	QUrl url("http://maps.googleapis.com/maps/api/staticmap");
	url.addQueryItem("center", QString("%1,%2").arg(latitude).arg(longitude));
	url.addQueryItem("zoom", QString("%1").arg(SETTINGS->imageMapZoom));
	url.addQueryItem("size", QString("%1x%1").arg(SETTINGS->imageMapSize));
	url.addQueryItem("maptype", SETTINGS->imageMapType.value().toLower());
	url.addQueryItem("language", "pl");
	url.addQueryItem("sensor", "false");
	manager.get(QNetworkRequest(url));
}

QPixmap GpsData::downloadCommonMap()
{
	if (!inCommonMode || allCoords.isEmpty())
		return QPixmap();

	QStringList points;
	foreach (QPointF point, allCoords.values())
		points << QString("%1,%2").arg(point.y()).arg(point.x());

	QUrl url("http://maps.googleapis.com/maps/api/staticmap");
	url.addQueryItem("size", "640x640");
	url.addQueryItem("maptype", SETTINGS->commonMapType.value().toLower());
	url.addQueryItem("language", "pl");
	url.addQueryItem("sensor", "false");
	url.addQueryItem("markers", points.join("|"));
	QNetworkReply *reply = manager.get(QNetworkRequest(url));
	QEventLoop loop;
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	QPixmap map;
	if (!map.loadFromData(reply->readAll()))
	{
		qDebug() << tr("Błąd otwierania mapy.");
		return QPixmap();
	}
	reply->deleteLater();
	return map;
}

void GpsData::finished(QNetworkReply *reply)
{
	if (inCommonMode)
		return;
	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << tr("Błąd pobierania mapy: %1 %2").arg(reply->error()).arg(reply->errorString());
		return;
	}
	QImage map;
	if (!map.loadFromData(reply->readAll()))
	{
		qDebug() << tr("Błąd otwierania mapy.");
		return;
	}
	imageCache = map.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	processMap(imageCache);
	reply->deleteLater();
}

void GpsData::processMap(QImage map)
{
	QColor color = SETTINGS->imageMapColor;
	QPoint center = QPoint(map.width() / 2.0, map.height() / 2.0);
	QPainter painter(&map);
	painter.setPen(QPen(color, 2));
	color.setAlpha(50);
	painter.setBrush(QBrush(color));
	painter.setRenderHint(QPainter::Antialiasing);

	if (hasDirection)
		painter.drawPie(map.rect().adjusted(-200, -200, 200, 200), -16 * (direction - 90 - 30), -16 * 60);
	else
		painter.drawEllipse(center, 7, 7);
	if (SETTINGS->imageMapCircle)
	{
		painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
		painter.drawImage(QPoint(0, 0), mask(map.size()));
	}
	painter.end();

	emit mapDownloaded(QPixmap::fromImage(map));
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
	painter.end();

	maskCache[sizeKey] = mask;
	return mask;
}
