#ifndef ABSTRACTMAPDOWNLOADER_H
#define ABSTRACTMAPDOWNLOADER_H

#include <QObject>
#include <QPointF>
#include <QImage>

class QNetworkReply;
class GpsData;

class AbstractMapDownloader : public QObject
{
	Q_OBJECT
	
public:
	typedef QHash<GpsData *, QPointF> Points;
	
	AbstractMapDownloader();
	
	virtual void finished(QNetworkReply *reply) = 0;
	virtual void downloadMap(QPointF point) = 0;
	virtual void downloadMap(Points points) = 0;
	
signals:
	void mapDownloaded(QImage map);
};

inline uint qHash(QPointF point)
{
	return point.toPoint().x() << 16 | point.toPoint().y();
}


#endif // ABSTRACTMAPDOWNLOADER_H
