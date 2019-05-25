#ifndef OVERLAY_H
#define OVERLAY_H

#include "overlayimage.h"
#include "exception.h"
#include <QString>
#include <QCoreApplication>

class Overlay : public AbstractMapDownloader
{
	Q_DECLARE_TR_FUNCTIONS(Overlay)

public:
	Overlay(QString absoluteFilePath) noexcept(false);
	~Overlay();

	QString toString() const;
	bool makeMap(GeoMap *map);
	DistancePair bestDistance(GeoMap *map) const;

	void writeThumbnail(QIODevice *device) const;

	static QString coordsToString(QPointF coords);
	static QString coordToQString(qreal coord, QString dir);

	static QPolygonF minimumBoundingBox(QList<QPointF> points);
	static void drawCompassRose(QPainter &painter, QRectF rect, qreal angle);

private:
	QString name, description;
	QList<OverlayImage *> images;
};

#endif // OVERLAY_H
