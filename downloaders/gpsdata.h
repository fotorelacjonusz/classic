#ifndef GPSDATA_H
#define GPSDATA_H

#include <QObject>
#include <QImage>

class SettingsDialog;
class AbstractMapDownloader;

class GpsData : public QObject
{
	Q_OBJECT
	typedef QHash<GpsData *, QPointF> Points;

public:
	GpsData(const int *const number, QString filePath);
	GpsData();
	GpsData(const int *const number, QDataStream &stream);
	~GpsData();
	QString toString() const;
	void serialize(QDataStream &stream) const;
	
public slots:
	void setPhotoSize(QSize size);
	void updateMapSize();
	void downloadMap();
	
signals:
	void mapReady(QImage map);

private:
	void init();

	const int *const number;
	bool hasPosition, hasDirection, isCommon;
	qreal latitude, longitude, direction;
	QSize photoSize;
	QSize mapSize;

	static Points allCoords;
	static QString binName;
	
	AbstractMapDownloader *googleDownloader, *tilesDownloader;
};

#endif // GPSDATA_H
