#ifndef GEOMAP_H
#define GEOMAP_H

#include <QObject>
#include <QPointF>
#include <QSize>
#include <QRect>
#include <QImage>

class GeoMap : public QObject
{
	Q_OBJECT
	
public:
	explicit GeoMap(QPointF coord, bool hasDirection, qreal direction, QSize size);
	explicit GeoMap(QList<QPointF> coords);
	
	void setImage(QImage image = QImage());
	QPoint coordToPoint(QPointF coord) const;
	
signals:
	void ready(QImage image);
	
protected:
	void processCommonMap(QImage &map) const;
	static void textBaloon(QPainter *painter, QPoint pos, QString text);
	void processMap(QImage &map) const;
	static QImage mask(QSize size);
	static QImage expanded(const QImage &map, QSize requestedSize);
	static void blur(QImage &image, QRect src, QRect dst);
	
public:
	const bool isCommon;
	const bool hasDirection;
	const qreal direction;
	const QSize size;
	const QList<QPointF> coords;
	const QList<QPointF> distinctCoords;
	const QRectF coordBox;
	QRect mapBox;
	
private:
	static QHash<int, QImage> maskCache;
};

#endif // GEOMAP_H
