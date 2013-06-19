#include "overlayimage.h"
#include "downloaders/geomap.h"
#include "settingsdialog.h"
#include <QMap>
#include <QPainter>
#include <QBuffer>
#include <QImageReader>
#include <QVector2D>
#include <math.h>

const qreal OverlayImage::R = 6371000;

template <class T> 
int sgn(T v)
{
    return (T(0) < v) - (v < T(0));
}

OverlayImage::OverlayImage(QDomElement groundOverlay, const QMap<QString, QByteArray> &files, bool isKmz) throw (Exception)
{
	QDomElement icon = groundOverlay.firstChildElement("Icon");
	QDomElement latLonBox = groundOverlay.firstChildElement("LatLonBox");
	name = groundOverlay.firstChildElement("name").text();

	(!icon.isNull() && !latLonBox.isNull()) OR_THROW(TR("Brak tagu w pliku kml"));
	m_href = icon.firstChildElement("href").text();

	qreal left = latLonBox.firstChildElement("west").text().toDouble();
	qreal top = latLonBox.firstChildElement("north").text().toDouble();
	qreal right = latLonBox.firstChildElement("east").text().toDouble();
	qreal bottom = latLonBox.firstChildElement("south").text().toDouble();

	lon0 = (left + right) / 2 * M_PI / 180;
	lat0 = (top + bottom) / 2 * M_PI / 180;

	rotation = latLonBox.firstChildElement("rotation").text().toDouble();

	poly << rotate(orthoProjection(QPointF(left, top))) << rotate(orthoProjection(QPointF(right, top)))
		 << rotate(orthoProjection(QPointF(right, bottom))) << rotate(orthoProjection(QPointF(left, bottom))); // clockwise
	poly << poly.first(); // close
	box = poly.boundingRect();

	files.contains(m_href) OR_THROW(TR("Brak pliku '%1' w archiwum kmz").arg(m_href));
	imageData = files.value(m_href);
	
	if (isKmz) // rotate imageData
	{
		QImage image;
		image.loadFromData(imageData) OR_THROW(TR("Nieudane ładowanie pliku z mapą, pamięć wyczerpana?"));
		
		QTransform transform;
		transform.rotate(-rotation);
		image = image.transformed(transform, Qt::SmoothTransformation);
		
		QBuffer buffer(&imageData);
		buffer.open(QIODevice::WriteOnly | QIODevice::Truncate);
		image.save(&buffer, "jpg") OR_THROW(TR("Nieudane zapisanie mapy do bufora, pamięć wyczerpana?"));
	}
	
	QBuffer buffer(&imageData);
	buffer.open(QIODevice::ReadOnly);
	QImageReader reader(&buffer);
	imageSize = reader.size();
}

bool OverlayImage::makeMap(GeoMap *map)
{
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

const QByteArray &OverlayImage::data() const
{
	return imageData;
}

const QString &OverlayImage::href() const
{
	return m_href;
}

const QString OverlayImage::toString() const
{
	return name;
}

qreal OverlayImage::distance(const GeoMap *map) const
{
	return distance(orthoProjection(map->first()), poly);
}

QImage OverlayImage::render(QPoint center, QSize size, qreal scale) const
{
	const QRect oversizeRect = centered(center, size * scale);
	const QRect clipRect = QRect(QPoint(), imageSize) & oversizeRect;
	const QSize scaledSize = clipRect.size() / scale;
	const QPoint rectTr = (clipRect.topLeft() - oversizeRect.topLeft()) / scale;
		
	QBuffer buffer(const_cast<QByteArray *>(&imageData));
	buffer.open(QIODevice::ReadOnly);
	QImageReader reader(&buffer);
	reader.setScaledSize(scaledSize);
	reader.setClipRect(clipRect);
	if (!reader.canRead())
	{
		qDebug() << "Can't read ImageReader";
		return QImage();
	}
	
	QImage result(size, QImage::Format_ARGB32);
	result.fill(Qt::black);
	QPainter(&result).drawImage(rectTr, reader.read());
	return result;
}

QPoint OverlayImage::coordToPoint(QPointF coord) const
{
	QPointF point = orthoProjection(coord);
	point.setY(point.y() * -1);
	Q_ASSERT(box.contains(point));
	
	point -= box.topLeft();
	point.rx() *= imageSize.width() / box.width();
	point.ry() *= imageSize.height() / box.height();
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

qreal OverlayImage::distance(QPointF p, QPointF a, QPointF b)
{
	const QPointF diff = b - a;
	const qreal A = -diff.y();
	const qreal B = diff.x();
	const qreal C = a.x() * diff.y() - a.y() * diff.x();
	const qreal lineDist = (A * p.x() + B * p.y() + C) / sqrt(A * A + B * B); // no abs, positive - left, negative - right
	const qreal pa = QVector2D(p - a).lengthSquared();
	const qreal pb = QVector2D(p - b).lengthSquared();
	const qreal ab = QVector2D(a - b).lengthSquared();
	
	if (pb > pa + ab) // point a is the nearest
		return sqrt(pa) * sgn(lineDist);
	else if (pa > pb + ab) // point b is the nearest
		return sqrt(pb) * sgn(lineDist);
	else // line strech is the nearest
		return lineDist;
}

qreal OverlayImage::distance(QPointF p, QPolygonF poly)
{
	if (!poly.isClosed() || poly.size() < 3)
		return 10000;
	QList<QPointF> list = poly.toList();
	qreal max = -10000;
	for (; list.size() >= 2; list.removeFirst())
		max = qMax(max, distance(p, list[0], list[1]));
	return max;
}
