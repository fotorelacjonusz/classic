#include "overlayimage.h"
#include "downloaders/geomap.h"
#include "settingsdialog.h"
#include <QMap>
#include <QPainter>
#include <math.h>

const qreal OverlayImage::R = 6371000;

OverlayImage::OverlayImage(QDomElement groundOverlay, const QMap<QString, QByteArray> &files, bool isKmz) throw (Exception)
{
	QDomElement icon = groundOverlay.firstChildElement("Icon");
	QDomElement latLonBox = groundOverlay.firstChildElement("LatLonBox");

	(!icon.isNull() && !latLonBox.isNull()) OR_THROW(TR("Brak tagu w pliku kml"));
	m_href = icon.firstChildElement("href").text();

	qreal left = latLonBox.firstChildElement("west").text().toDouble();
	qreal top = latLonBox.firstChildElement("north").text().toDouble();
	qreal right = latLonBox.firstChildElement("east").text().toDouble();
	qreal bottom = latLonBox.firstChildElement("south").text().toDouble();

	lon0 = (left + right) / 2 * M_PI / 180;
	lat0 = (top + bottom) / 2 * M_PI / 180;

	rotation = latLonBox.firstChildElement("rotation").text().toDouble();// / 180.0 * M_PI;

	poly << rotate(orthoProjection(QPointF(left, top))) << rotate(orthoProjection(QPointF(right, top)))
		 << rotate(orthoProjection(QPointF(right, bottom))) << rotate(orthoProjection(QPointF(left, bottom)));
	box = poly.boundingRect();

//	qDebug() << box;

//	box.isValid() OR_THROW(TR("Niepoprawne koordynaty"));
	files.contains(m_href) OR_THROW(TR("Brak pliku '%1' w archiwum kmz").arg(m_href));
	overlayImage.loadFromData(files[m_href]) OR_THROW(TR("Nieudane ładowanie pliku z mapą"));
	
	if (isKmz)
	{
		QTransform transform;
		transform.rotate(-rotation);
		overlayImage = overlayImage.transformed(transform, Qt::SmoothTransformation);
	}
}

bool OverlayImage::makeMap(GeoMap *map)
{
//	if (!isValid())
//		return false;
	foreach (QPointF coord, map->distinctCoords)
		if (!poly.containsPoint(orthoProjection(coord), Qt::OddEvenFill))
			return false;
	
	if (!map->isCommon)
		map->setImage(render(coordToPoint(map->first()), map->size, 22 - SETTINGS->imageMapZoom));
	else if (map->isSingle)
		map->setImage(render(coordToPoint(map->first()), maxSize(), 4), QSize(0, 0));
	else
	{
		QRect rect(coordToPoint(map->coordBox.bottomLeft()), coordToPoint(map->coordBox.topRight()));
		qreal scale = qMax((rect.width() / (maxSize().width() - 2.0 * margin)), (rect.height() / (maxSize().height() - 2.0 * margin)));
		map->setImage(render(rect.center(), maxSize(), scale), rect.size() / scale);
	}
	return true;
}

const QImage &OverlayImage::image() const
{
	return overlayImage;
}

const QString &OverlayImage::href() const
{
	return m_href;
}

QImage OverlayImage::render(QPoint center, QSize size, qreal scale) const
{
	QImage mapCopy(size * scale, QImage::Format_ARGB32);
	mapCopy.fill(Qt::black);
	QPainter(&mapCopy).drawImage(QPoint(0, 0), overlayImage, centered(center, size * scale));
	return mapCopy.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPoint OverlayImage::coordToPoint(QPointF coord) const
{
	QPointF point = orthoProjection(coord);
	point.setY(point.y() * -1);
	Q_ASSERT(box.contains(point));
	
	point -= box.topLeft();
	point.rx() *= overlayImage.width() / box.width();
	point.ry() *= overlayImage.height() / box.height();
	return point.toPoint();
}

QPointF OverlayImage::rotate(QPointF point) const
{
	qreal alpha = atan2(point.y(), point.x());
	qreal r = sqrt(point.x() * point.x() + point.y() * point.y());
	qreal beta = alpha + rotation * M_PI / 180;
	return QPointF(cos(beta) * r, sin(beta) * r);
}

QPointF OverlayImage::orthoProjection(QPointF coord) const
{
	qreal lon = coord.x() * M_PI / 180;
	qreal lat = coord.y() * M_PI / 180;
	qreal x = R * cos(lat) * sin(lon - lon0);
	qreal y = R * (cos(lat0) * sin(lat) - sin(lat0) * cos(lat) * cos(lon - lon0));
	return QPointF(x, y);
}

QPointF OverlayImage::inverseOrthoProjection(QPointF point) const
{
	qreal x = point.x();
	qreal y = point.y();
	qreal ro = sqrt(x * x + y * y);
	qreal c = asin(ro / R);
	qreal lat = asin(cos(c) * sin(lat0) + (y * sin(c) * cos(lat0) / ro));
	qreal lon = lon0 + atan2(x * sin(c), (ro * cos(lat0) * cos(c) - y * sin(lat0) * sin(c)));
	lon *= 180 / M_PI;
	lat *= 180 / M_PI;
	return QPointF(lon, lat);
}
