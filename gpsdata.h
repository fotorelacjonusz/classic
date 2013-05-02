#ifndef GPSDATA_H
#define GPSDATA_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QNetworkDiskCache>

#include "googlemapsdownloader.h"
#include "tilesdownloader.h"

class SettingsDialog;

class GpsData : public QObject
{
	Q_OBJECT

public:
	GpsData(const int *const number, QString filePath);
	GpsData();
	GpsData(const int *const number, QDataStream &stream);
	~GpsData();
	QString toString() const;
	void serialize(QDataStream &stream) const;

	const int *const number;
public slots:
	void downloadMap();
//	QPixmap downloadCommonMap();

private slots:
	void mapDownloaded(QImage map);

signals:
	void mapReady(QPixmap map);

protected:
	void init();
	
	void processMap(QImage &map);
	static QImage mask(QSize size);

	bool hasPosition, hasDirection, inCommonMode;
	qreal latitude, longitude, direction;
//	QNetworkAccessManager manager;
//	QNetworkDiskCache cache;
//	QImage imageCache;
	static QHash<int, QImage> maskCache;
	static QHash<GpsData *, QPointF> allCoords;
	static QString binName;
	
	GoogleMapsDownloader googleDownloader;
	TilesDownloader tilesDownloader;
};

#endif // GPSDATA_H
