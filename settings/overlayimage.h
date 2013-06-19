#ifndef OVERLAYIMAGE_H
#define OVERLAYIMAGE_H

#include "downloaders/abstractmapdownloader.h"
#include "exception.h"

#include <QRectF>
#include <QPolygonF>
#include <QImage>
#include <QDomElement>

/* 
 * This class has three coordinate systems:
 * 1. Geographic coordinates (longitude, latitude), aka coordinates, coord    (positive y points up)
 * 2. Position in meters with center (0, 0) at image center aka position, pos (positive y points up)
 * 3. Pixel position in image, same as Qt's pixel coordinate system aka point (positive y points down)
 * Thus coord(lon0, lat0), pos(0, 0), and point(overlayImage.rect().center) are the same center point
 */
class OverlayImage : public AbstractMapDownloader
{
public:
	OverlayImage(QDomElement groundOverlay, const QMap<QString, QByteArray> &files, bool isKmz) throw (Exception);
	bool makeMap(GeoMap *map);
	const QByteArray &data() const;
	const QString &href() const;
	const QString toString() const;
	
	// distance from first point to the overlay (nearest edge or corner) in meters, negative numbers are inside
	qreal distance(const GeoMap *map) const;
	
protected:
	// render part of the overlay with given size and center point, covering size * scale area
	QImage render(QPoint center, QSize size, qreal scale) const;
	// map (coord -> point)
	QPoint coordToPoint(QPointF coord) const;
	// rotate pos around (lon0, lat0) coord - i.e. (0, 0), by rotation degrees (meters -> meters)
	QPointF rotate(QPointF point) const;
	// orthographic projection (coord -> meters) - pl_PL"rzut prostokątny"
	QPointF orthoProjection(QPointF coord) const;
	// inverse orthographic projection (meters -> coord) - pl_PL"odwrotny rzut prostokątny"
	QPointF inverseOrthoProjection(QPointF point) const;
	// distance from point p to line strech between a and b, positive - left, negative - right
	static qreal distance(QPointF p, QPointF a, QPointF b);
	// distance from point p to lines of the polygon, the less, the more inside the polygon (clockwise), returns the worst (max)
	static qreal distance(QPointF p, QPolygonF poly);

private:
	QString name, m_href;
	// map rectangle (including unused black areas) in meters with center (0, 0) at image center corresponding to image.rect()
	QRectF box;
	// actuall map area (excluding unused black areas) in meters with center (0, 0) at image center
	QPolygonF poly;
	// overlayImage rotation in degrees around center
	qreal rotation;
	// center
	qreal lat0, lon0;
	// radius of the earth [m]
	static const qreal R; 
	// image data for QImageReader
	QByteArray imageData;
	// image size
	QSize imageSize;
};

#endif // OVERLAYIMAGE_H
