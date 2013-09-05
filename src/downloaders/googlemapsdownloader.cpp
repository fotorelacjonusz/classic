#include "googlemapsdownloader.h"
#include "settings/settingsdialog.h"
#include "throttlednetworkmanager.h"
#include "geomap.h"

#include <QNetworkRequest>
#include <QNetworkReply>

bool GoogleMapsDownloader::makeMap(GeoMap *map)
{
	QStringList markers;
	foreach (QPointF point, map->coords.values())
		markers << QString("%1,%2").arg(point.y(), 0, 'f', 8).arg(point.x(), 0, 'f', 8);
	
	QUrl url("http://maps.googleapis.com/maps/api/staticmap");
	url.addQueryItem("sensor", "false");
	url.addQueryItem("language", "pl");
	if (map->isCommon)
	{
		url.addQueryItem("size", "640x640");
		url.addQueryItem("maptype", SETTINGS->mapTypeToString(SETTINGS->commonMapType));
		url.addQueryItem("markers", markers.join("|"));
	}
	else
	{
		url.addQueryItem("size", QString("%1x%2").arg(map->size.width()).arg(map->size.height()));
		url.addQueryItem("maptype", SETTINGS->mapTypeToString(SETTINGS->imageMapType));
		url.addQueryItem("zoom", QString("%1").arg(SETTINGS->imageMapZoom));
		url.addQueryItem("center", markers.first());
	}
	
	static ThrottledNetworkManager manager(100);
	manager.get(QNetworkRequest(url), this);
	maps[url] = map;
	return true;
}

void GoogleMapsDownloader::finished(QNetworkReply *reply)
{
	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << QObject::tr("Błąd pobierania mapy: %1 %2").arg(reply->error()).arg(reply->errorString());
		return;
	}
	QImage image;
	if (!image.loadFromData(reply->readAll()))
	{
		qDebug() << QObject::tr("Błąd otwierania mapy.");
		return;
	}
	
	reply->deleteLater();
	
	if (maps.contains(reply->url()))
		maps.take(reply->url())->setImage(image);
}
