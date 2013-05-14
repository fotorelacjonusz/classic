#ifndef ABSTRACTMAPDOWNLOADER_H
#define ABSTRACTMAPDOWNLOADER_H

#include <QObject>
#include <QPointF>
#include <QRect>

class QNetworkReply;
class GpsData;
class GeoMap;

class AbstractMapDownloader 
{
protected:
	AbstractMapDownloader();
	
public:
	virtual ~AbstractMapDownloader();
	virtual void finished(QNetworkReply *reply);
	virtual bool makeMap(GeoMap *map) = 0;
};

inline uint qHash(QPointF point)
{
	return point.toPoint().x() << 16 | point.toPoint().y();
}

QRect centered(const QPoint &center, const QSize &size);
QRect centered(const QPoint &center, const int size);

QRect resized(const QRect &rect, const qreal factor);
QRect resized(const QRect &rect, QSize size);

#endif // ABSTRACTMAPDOWNLOADER_H
