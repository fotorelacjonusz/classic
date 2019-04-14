#include "overlayimage.h"
#include "downloaders/geomap.h"
#include "settingsdialog.h"
#include <QMap>
#include <QPainter>
#include <QBuffer>
#include <QImageReader>
#include <QVector2D>
#include <cmath>

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
	m_name = groundOverlay.firstChildElement("name").text();

	(!icon.isNull() && !latLonBox.isNull()) OR_THROW(TR("Brak tagu w pliku kml"));
	m_href = icon.firstChildElement("href").text();

	const qreal left = latLonBox.firstChildElement("west").text().toDouble();
	const qreal top = latLonBox.firstChildElement("north").text().toDouble();
	const qreal right = latLonBox.firstChildElement("east").text().toDouble();
	const qreal bottom = latLonBox.firstChildElement("south").text().toDouble();

	lon0 = (left + right) / 2 * M_PI / 180;
	lat0 = (top + bottom) / 2 * M_PI / 180;

	rotation = latLonBox.firstChildElement("rotation").text().toDouble(); // if "rotation" tag doesn't exist, 0 is ok

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

	const qreal hPpm = distance(left, top, right, top, &OverlayImage::coordToPoint) /  // [px] /
					   distance(left, top, right, top, &OverlayImage::orthoProjection); // [m]
	const qreal vPpm = distance(left, top, left, bottom, &OverlayImage::coordToPoint) /
					   distance(left, top, left, bottom, &OverlayImage::orthoProjection);
	ppm = sqrt(hPpm * vPpm);
//	qDebug() << name << hPpm << vPpm << ppm;
}

OverlayImage::OverlayImage(QRectF latLongBox, QSize maxSize)
{
	lon0 = latLongBox.center().x() * M_PI / 180;
	lat0 = latLongBox.center().y() * M_PI / 180;

	rotation = 0.0;

	poly << orthoProjection(latLongBox.topLeft()) << orthoProjection(latLongBox.topRight())
		 << orthoProjection(latLongBox.bottomRight()) << orthoProjection(latLongBox.bottomLeft()); // clockwise
	poly << poly.first(); // close
	box = poly.boundingRect();

	QSizeF size = box.size();
	size.scale(maxSize, Qt::KeepAspectRatio);
	imageSize = size.toSize();
}

bool OverlayImage::makeMap(GeoMap *map)
{
	foreach (QPointF coord, map->distinctCoords)
		if (!poly.containsPoint(orthoProjection(coord), Qt::OddEvenFill))
			return false;

	if (!map->isCommon)
		map->setImage(render(coordToPoint(map->first()), map->size, 22 - SETTINGS->imageMapZoom, true));
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

QString OverlayImage::megaToString(qreal number)
{
	return QString::number(number / 1024.0 / 1024.0, 'f', 1);
}

QString OverlayImage::name() const
{
	return m_name;
}

QString OverlayImage::tech() const
{
	const QPointF p0 = posToPointF(poly[0]);
	const QPointF p1 = posToPointF(poly[1]);
	const QPointF p2 = posToPointF(poly[2]);
	const qreal area = QVector2D(p0 - p1).length() * QVector2D(p1 - p2).length();

	return QString("%1 MB, %2 MP, %3 ppm").arg(megaToString(imageData.size())).arg(megaToString(area)).arg(ppm, 0, 'f', 1);
}

QSize OverlayImage::size() const
{
	return imageSize;
}

QPolygonF OverlayImage::coordPolygon() const
{
	QPolygonF result;
	foreach (QPointF pos, poly)
		result << inverseOrthoProjection(pos);
	return result;
}

QPointF OverlayImage::coordCenter() const
{
	return QPointF(lat0, lon0);
}

QImage OverlayImage::render(QPoint center, QSize size, qreal scale, bool includePpm) const
{
	// I assume pixel per meter of a detailed satellite image (or a blueprint) is 10ppm, for which the following code doesn't change the scale
	// the bigger the ppm, the more pixels should be copied
	if (includePpm)
		scale *= ppm / 10.0;

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

QImage OverlayImage::render(QSize scaledSize)
{
	QBuffer buffer(const_cast<QByteArray *>(&imageData));
	buffer.open(QIODevice::ReadOnly);
	QImageReader reader(&buffer);
	reader.setScaledSize(scaledSize);
	if (!reader.canRead())
	{
		qDebug() << "Can't read ImageReader";
		return QImage();
	}
	return reader.read();
}

QPointF OverlayImage::posToPointF(QPointF pos) const
{
	pos.setY(pos.y() * -1);
//	Q_ASSERT(box.contains(pos));

	pos -= box.topLeft();
	pos.rx() *= imageSize.width() / box.width();
	pos.ry() *= imageSize.height() / box.height();
	return pos;
}

QPoint OverlayImage::coordToPoint(QPointF coord) const
{
	return posToPointF(orthoProjection(coord)).toPoint();
}

QPointF OverlayImage::rotate(QPointF point) const
{
	const qreal alpha = atan2(point.y(), point.x());
	const qreal r = sqrt(point.x() * point.x() + point.y() * point.y());
	const qreal beta = alpha + rotation * M_PI / 180;
	return QPointF(cos(beta) * r, sin(beta) * r);
}

QPointF OverlayImage::orthoProjection(QPointF coord) const
{
	const qreal lon = coord.x() * M_PI / 180;
	const qreal lat = coord.y() * M_PI / 180;
	const qreal x = R * cos(lat) * sin(lon - lon0);
	const qreal y = R * (cos(lat0) * sin(lat) - sin(lat0) * cos(lat) * cos(lon - lon0));
	return QPointF(x, y);
}

QPointF OverlayImage::inverseOrthoProjection(QPointF pos) const
{
	const qreal x = pos.x();
	const qreal y = pos.y();
	const qreal ro = sqrt(x * x + y * y);
	const qreal c = asin(ro / R);
	const qreal lat = asin(cos(c) * sin(lat0) + (y * sin(c) * cos(lat0) / ro));
	const qreal lon = lon0 + atan2(x * sin(c), (ro * cos(lat0) * cos(c) - y * sin(lat0) * sin(c)));
	return QPointF(lon * 180 / M_PI, lat * 180 / M_PI);
}

qreal OverlayImage::distance(QPointF p, QLineF l)
{
	const qreal A = -l.dy();
	const qreal B = l.dx();
	const qreal C = l.x1() * l.dy() - l.y1() * l.dx();
	const qreal lineDist = (A * p.x() + B * p.y() + C) / sqrt(A * A + B * B); // no abs, positive - left, negative - right
	const qreal pa = QVector2D(p      - l.p1()).lengthSquared();
	const qreal pb = QVector2D(p      - l.p2()).lengthSquared();
	const qreal ab = QVector2D(l.p1() - l.p2()).lengthSquared();

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
		max = qMax(max, distance(p, QLineF(list[0], list[1])));
	return max;
}

qreal OverlayImage::distance(const GeoMap *map) const
{
	return distance(orthoProjection(map->first()), poly) * ppm;
}

template <typename T>
qreal OverlayImage::distance(qreal x1, qreal y1, qreal x2, qreal y2, T transform) const
{
	return QVector2D((this->*transform)(QPointF(x1, y1)) - (this->*transform)(QPoint(x2, y2))).length();
}
