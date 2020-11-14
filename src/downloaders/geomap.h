#pragma once

#include <QObject>
#include <QPointF>
#include <QSize>
#include <QRect>
#include <QImage>
#include <QMap>

class GeoMap : public QObject
{
	Q_OBJECT

public:
	typedef QMap<int, QPointF> CoordMap;

	explicit GeoMap(QPointF coord, bool hasDirection, qreal direction, QSize size);
	explicit GeoMap(CoordMap coords);

	static CoordMap makeMap(const QPointF &point);
	QPointF first() const;

	// set map image where rectangle centered on image center of size size coresponds to coordBox for common map
	// for single position maps size is ignored
	void setImage(QImage image = QImage(), QSize size = QSize());
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
	const CoordMap coords;
	const QList<QPointF> distinctCoords;
	const QRectF coordBox;
	const bool isSingle;

private:
	QRect mapBox;
	static QHash<int, QImage> maskCache;
};
