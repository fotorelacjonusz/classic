#ifndef GPSDATA_H
#define GPSDATA_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QPixmap>

class SettingsDialog;

class GpsData : public QObject
{
	Q_OBJECT

public:
	GpsData(QString filePath);
	GpsData();
	GpsData(QDataStream &stream);
	~GpsData();
	QString toString() const;
	void serialize(QDataStream &stream) const;

public slots:
	void downloadMap();
	QPixmap downloadCommonMap();

signals:
	void mapDownloaded(QPixmap map);

private slots:
	void finished(QNetworkReply *reply);

private:
	void processMap(QImage map);
	static QImage mask(QSize size);

	bool hasPosition, hasDirection, inCommonMode;
	qreal latitude, longitude, direction;
	QNetworkAccessManager manager;
	QImage imageCache;
	static QMap<int, QImage> maskCache;
	static QMap<GpsData *, QPointF> allCoords;
	static QString binName;
};

#endif // GPSDATA_H
