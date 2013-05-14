#ifndef OVERLAY_H
#define OVERLAY_H

#include "downloaders/abstractmapdownloader.h"

#include <QString>
#include <QRectF>
#include <QPolygonF>
#include <QImage>

class Overlay : public AbstractMapDownloader
{
public:
	Overlay(QString absoluteFilePath);
	bool isValid() const;
	bool makeMap(GeoMap *map);
	QString toString() const;

private:
	// rotate point around (lon0, lat0) point - i.e. (0, 0), by _rotation_ degrees (obrót wokół punktu lon0, lat0, czyli 0, 0 o rotation stopni)
	QPointF rotate(QPointF point) const;
	// orthographic projection (rzut prostokątny)
	QPointF orthoProjection(QPointF coords) const;
	// inverse orthographic projection (odwrotny rzut prostokątny)
	QPointF inverseOrthoProjection(QPointF point) const;

	bool setError(QString error);

	QString name, href, error;
	// map rectangle (including unused black areas)
	QRectF box;
	// actuall map area
	QPolygonF poly;
	qreal rotation;
	qreal lat0, lon0;
	static const qreal R; // radius of the earth [m]
	QImage overlayImage;
};

#endif // OVERLAY_H
