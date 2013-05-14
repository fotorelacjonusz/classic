#include "geomap.h"
#include "abstractmapdownloader.h"
#include "settings/settingsdialog.h"

#include <QSet>
#include <QPolygonF>
#include <QPainter>
#include <QLabel>
#include <QGraphicsBlurEffect>

QHash<int, QImage> GeoMap::maskCache;

GeoMap::GeoMap(QPointF coord, bool hasDirection, qreal direction, QSize size):
	isCommon(false), hasDirection(hasDirection), direction(direction), size(size),
	coords(QList<QPointF>() << coord),
	distinctCoords(coords.toSet().toList()),
	coordBox(QPolygonF(coords.toVector()).boundingRect())
{
}

GeoMap::GeoMap(QList<QPointF> coords):
	isCommon(true), hasDirection(false), direction(0), 
	coords(coords),
	distinctCoords(coords.toSet().toList()),
	coordBox(QPolygonF(coords.toVector()).boundingRect())
{
}

void GeoMap::setImage(QImage image)
{
	if (!image.isNull())
	{
		if (image.format() != QImage::Format_ARGB32 &&
			image.format() != QImage::Format_ARGB32_Premultiplied)
			image = image.convertToFormat(QImage::Format_ARGB32);
		
		if (isCommon)
			processCommonMap(image);
		else
			processMap(image);
	}

	emit ready(image);
	deleteLater();
}

QPoint GeoMap::coordToPoint(QPointF coord) const
{
	const qreal sx = coordBox.width()  ? mapBox.width()  / coordBox.width()  : 1.0;
	const qreal sy = coordBox.height() ? mapBox.height() / coordBox.height() : 1.0;
	
	return QPoint((coord.x() - coordBox.left()) * sx, (coordBox.bottom() - coord.y()) * sy) + mapBox.topLeft();
}

void GeoMap::processCommonMap(QImage &map) const
{
	QPainter painter(&map);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.setFont(QFont("Arial", 12));

	QHash<QPointF, QStringList> merged;
	for (int i = 0; i < coords.size(); ++i)
		merged[coords[i]].append(QString::number(i + SETTINGS->startingNumber));
	
	for (QHash<QPointF, QStringList>::Iterator i = merged.begin(); i != merged.end(); ++i)
		textBaloon(&painter, coordToPoint(i.key()), SETTINGS->numberImages ? i.value().join(", ") : "✖"); ; // "●"
}

void GeoMap::textBaloon(QPainter *painter, QPoint pos, QString text)
{
	QRectF bounding = painter->boundingRect(QRectF(0, 0, 1000, 1000), text);
	bounding.setWidth(qMax(bounding.width() + 8, 20.0));
	
	QPainterPath path;
	path.moveTo(5, bounding.height() - 6);
	path.lineTo(0, bounding.height() + 4);
	path.lineTo(20, bounding.height() - 6);
	path.closeSubpath();
	QPainterPath roundedRect;
	roundedRect.addRoundedRect(bounding, 5, 5);
	path |= roundedRect;
	path.translate(pos - QPoint(0, path.boundingRect().height()));

	painter->fillPath(path, QBrush(Qt::white));
	painter->strokePath(path, QPen(Qt::black));
	painter->drawText(bounding.translated(path.boundingRect().topLeft()), text, Qt::AlignHCenter | Qt::AlignVCenter);
}

void GeoMap::processMap(QImage &map) const
{
	// if map was clipped, e.g. because of google's size constraint of 640px, we need to expand
	if (map.size() != size) 
		map = expanded(map, size);
		
	QColor color = SETTINGS->imageMapColor;
	color.setAlpha(200);
	QPainter painter(&map);
//	painter.setPen(QPen(color, 2));
	painter.setPen(QPen(Qt::transparent));
	painter.setRenderHint(QPainter::Antialiasing);

	if (hasDirection)
	{
		QRadialGradient gradient(map.rect().center(), SETTINGS->imageMapSize * 0.5);
		gradient.setColorAt(0.0, color);
		gradient.setColorAt(1.0, Qt::transparent);
		painter.setBrush(QBrush(gradient));
		painter.drawPie(centered(map.rect().center(), SETTINGS->imageMapSize), -16 * (direction - 90 - 30), -16 * 60);
	}
	else
	{
		painter.setBrush(QBrush(color));
		painter.drawEllipse(map.rect().center(), 7, 7);
	}
	
	if (SETTINGS->imageMapCircle && SETTINGS->imageMapCorner < SettingsDialog::Expand)
	{
		painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
		painter.drawImage(QPoint(0, 0), mask(map.size()));
	}
}

QImage GeoMap::mask(QSize size)
{
	Q_ASSERT(size.width() == size.height());
	int sizeKey = size.width();

	if (maskCache.contains(sizeKey))
		return maskCache[sizeKey];

	QImage mask(size, QImage::Format_ARGB32);
	mask.fill(Qt::transparent);
	QPainter painter(&mask);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(QBrush(Qt::black));
	painter.drawPie(mask.rect(), 0, 16 * 360);

	return maskCache[sizeKey] = mask;
}

QImage GeoMap::expanded(const QImage &map, QSize requestedSize)
{
	QImage result(requestedSize, QImage::Format_RGB32);
	
	QRect mapRect = centered(result.rect().center(), map.size());
	QPainter(&result).drawImage(mapRect, map);

	QVector<QRect> outer = (QRegion(result.rect()) - QRegion(mapRect)).rects();
	QVector<QRect> inner = ((QRegion(resized(outer[0], 3)) + QRegion(resized(outer[1], 3))) & QRegion(mapRect)).rects();
	blur(result, inner[0], outer[0]);
	blur(result, inner[1], outer[1]);
	return result;
}

void GeoMap::blur(QImage &image, QRect src, QRect dst)
{
//	qDebug() << src << dst;
	
	int count = 0, sumR = 0, sumG = 0, sumB = 0;
	for (int x = src.left(); x <= src.right(); x += 10)
		for (int y = src.top(); y <= src.bottom(); y += 10, ++count)
		{
			sumR += qRed(image.pixel(x, y));
			sumG += qGreen(image.pixel(x, y));
			sumB += qBlue(image.pixel(x, y));
		}
	QColor avg(sumR / count, sumG / count, sumB / count);
	QPainter(&image).fillRect(dst, avg);
	
    QLabel label;
	QGraphicsBlurEffect *effect = new QGraphicsBlurEffect();
	effect->setBlurRadius(10);
	label.setGraphicsEffect(effect);
    label.setPixmap(QPixmap::fromImage(image.copy(src)));
    label.render(&image, dst.topLeft());
}

