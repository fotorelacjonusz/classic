#include "googlemapsdownloader.h"
#include "settingsdialog.h"
#include "throttlednetworkmanager.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

GoogleMapsDownloader::GoogleMapsDownloader()
{
}

void GoogleMapsDownloader::downloadMap(QPointF point)
{
	QUrl url("http://maps.googleapis.com/maps/api/staticmap");
	url.addQueryItem("center", QString("%1,%2").arg(point.y()).arg(point.x()));
	url.addQueryItem("zoom", QString("%1").arg(SETTINGS->imageMapZoom));
	url.addQueryItem("size", QString("%1x%1").arg(SETTINGS->imageMapSize));
	url.addQueryItem("maptype", SETTINGS->mapTypeToString(SETTINGS->imageMapType)); // SETTINGS->imageMapType.value().toLower());
	url.addQueryItem("language", "pl");
	url.addQueryItem("sensor", "false");
	get(url);
}

void GoogleMapsDownloader::downloadMap(Points points)
{
	QStringList markers;
	foreach (QPointF point, points)
		markers << QString("%1,%2").arg(point.y()).arg(point.x());

	QUrl url("http://maps.googleapis.com/maps/api/staticmap");
	url.addQueryItem("size", "640x640");
	url.addQueryItem("maptype", SETTINGS->mapTypeToString(SETTINGS->commonMapType)); //.value().toLower());
	url.addQueryItem("language", "pl");
	url.addQueryItem("sensor", "false");
	url.addQueryItem("markers", markers.join("|"));
	get(url);
//	QNetworkReply *reply = manager.get(QNetworkRequest(url));
//	QEventLoop loop;
//	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
//	loop.exec();

//	QPixmap map;
//	if (!map.loadFromData(reply->readAll()))
//	{
//		qDebug() << tr("Błąd otwierania mapy.");
//		return QPixmap();
//	}
//	reply->deleteLater();
//	return map;
}

void GoogleMapsDownloader::finished(QNetworkReply *reply)
{
//	if (inCommonMode)
//		return;
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
//	imageCache = ;
//	processMap(map.convertToFormat(QImage::Format_ARGB32_Premultiplied));
	reply->deleteLater();
	emit mapDownloaded(map);
}

void GoogleMapsDownloader::get(QUrl url)
{
	static ThrottledNetworkManager manager(100);
	manager.get(QNetworkRequest(url), this);
}
