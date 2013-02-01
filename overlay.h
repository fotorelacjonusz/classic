#ifndef OVERLAY_H
#define OVERLAY_H

#include <QString>
#include <QRectF>
#include <QPolygonF>
#include <QPixmap>
#include <QVector2D>

class Overlay
{
public:
	QString name, href, error;

	Overlay(QString absoluteFilePath);
	bool isValid() const;
	bool contains(qreal lon, qreal lat) const;
	QPixmap makeMap(qreal lon, qreal lat) const;

private:
	// rotation around (lon0, lat0) point - i.e. (0, 0), by _rotation_ degrees (obrót wokół punktu lon0, lat0, czyli 0, 0 o rotation stopni)
	QPointF rotate(QPointF point) const;
	// orthographic projection (rzut prostokątny)
	QPointF orthoProjection(qreal lon, qreal lat) const;
	// inverse orthographic projection (odwrotny rzut prostokątny)
	QPointF inverseOrthoProjection(QPointF point) const;

	bool setError(QString error);

	QRectF box;
	QPolygonF poly;
	qreal rotation;
	qreal lat0, lon0;
	static const qreal R; // radius of the earth [m]
	QPixmap map;
};

#endif // OVERLAY_H
