#pragma once

#include <QObject>
#include <QImage>

class SettingsDialog;
class AbstractMapDownloader;
class ExifValue;
class ExifImageHeader;
class GpxDialog;

class GpsData : public QObject
{
	Q_OBJECT
	typedef QHash<GpsData *, QPointF> Points;

public:
	GpsData(QIODevice *image, const int *const number);
	GpsData();
	~GpsData();
	void writeExif(QIODevice *device) const;
	QString toString() const;
	void serialize(QDataStream &stream) const;

	bool setPosition(GpxDialog *gpxDialog);
	void removePosition();

	QMatrix reverseMatrixForOrientation() const;
public slots:
	void setPhotoSize(QSize size);
	void updateMapSize();
	void downloadMap();

signals:
	void mapReady(QImage map);

private:
	const int *const number;
	bool hasPosition, hasDirection;
	const bool isCommon;
	qreal latitude, longitude, direction;
	QSize photoSize;
	QSize mapSize;

	static Points allCoords;
	static QString binName;

	AbstractMapDownloader *googleDownloader, *tilesDownloader;
	ExifImageHeader *exifHeader;
};
