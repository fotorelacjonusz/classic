#include "overlay.h"
#include "application.h"

#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

#include "messagehandler.h"
#include "myexif/exifimageheader.h"

#include <QDomDocument>
#include <QDomNode>
#include <QMap>
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QtAlgorithms>
#include <QVector2D>
#include <QMatrix4x4>
#include <QtMath>

#define DOC_KML "doc.kml"

Overlay::Overlay(QString absoluteFilePath) noexcept(false)
{
	quint32 thumbnailSize;
	QFile file(absoluteFilePath);
	file.open(QIODevice::ReadOnly);
	if (absoluteFilePath.endsWith(".jpg")) // skip thumbnail
	{
		file.seek(file.size() - sizeof(thumbnailSize));
		QDataStream(&file) >> thumbnailSize; // BigEndian by default
		if (file.size() < thumbnailSize)
			THROW(tr("To nie jest plik podkładu mapowego."));
		file.seek(thumbnailSize);
	}

	QMap<QString, QByteArray> files;
	{
//		QuaZip kmz(absoluteFilePath);
		QuaZip kmz(&file);

		(Suppress(), kmz.open(QuaZip::mdUnzip)) OR_THROW(tr("kmz.open(): %1").arg(kmz.getZipError()));

		QuaZipFile file(&kmz);
		for (bool f = kmz.goToFirstFile(); f; f = kmz.goToNextFile())
		{
			file.open(QIODevice::ReadOnly) OR_THROW(tr("file.open(): %1").arg(file.getZipError()));
			files[file.getActualFileName()] = file.readAll();
			file.close();
			file.getZipError() == UNZ_OK OR_THROW(tr("file.close(): %1").arg(file.getZipError()));
		}

		kmz.close();
		kmz.getZipError() == UNZ_OK OR_THROW(tr("kmz.close(): %1").arg(kmz.getZipError()));
	}

//	qDebug() << files.keys();
	files.contains(DOC_KML) OR_THROW(tr("Brak pliku '%1'").arg(DOC_KML));
	bool isKmz = absoluteFilePath.endsWith(".kmz");

	QDomDocument doc;
	doc.setContent(files[DOC_KML]) OR_THROW(tr("Niepoprawna składnia kml"));
	QDomElement kml = doc.documentElement();
	QDomElement firstChild = kml.firstChild().toElement();
	(!kml.isNull() && !firstChild.isNull()) OR_THROW(tr("Brak tagu kml i/lub podrzędnego w pliku kml"));

	name = firstChild.firstChildElement("name").text();
	description = firstChild.firstChildElement("description").text();

	if (firstChild.tagName() == "GroundOverlay")
		images << new OverlayImage(firstChild, files, isKmz);
	else if (firstChild.tagName() == "Folder")
	{
		QDomNodeList overlays = firstChild.elementsByTagName("GroundOverlay");
		for (int i = 0; i < overlays.size(); ++i)
			images << new OverlayImage(overlays.at(i).toElement(), files, isKmz);
//		writeThumbnail(0);
	}
	else
		THROW(tr("Brak tagu Folder lub GroundOverlay"));

	if (isKmz)
	{
		QString jpgFilePath = absoluteFilePath.section(".", 0, -2) + ".jpg";
		QFile jpg(jpgFilePath);
		jpg.open(QIODevice::ReadWrite);
		writeThumbnail(&jpg);
		thumbnailSize = jpg.size();

		{
			QuaZip kmr(&jpg);
			(Suppress(), kmr.open(QuaZip::mdCreate)) OR_THROW(tr("kmr.open(): %1").arg(kmr.getZipError()));

			QuaZipFile outFileKml(&kmr);
			outFileKml.open(QIODevice::WriteOnly, QuaZipNewInfo(DOC_KML, absoluteFilePath)) OR_THROW(tr("outFileKml.open(): %1").arg(outFileKml.getZipError()));
			outFileKml.write(doc.toByteArray());
			outFileKml.close();
			outFileKml.getZipError() == UNZ_OK OR_THROW(tr("outFileKml.close(): %1").arg(outFileKml.getZipError()));

			foreach (OverlayImage *image, images)
			{
				QuaZipFile outFileMap(&kmr);
				outFileMap.open(QIODevice::WriteOnly, QuaZipNewInfo(image->href(), absoluteFilePath)) OR_THROW(tr("outFileMap.open(): %1").arg(outFileMap.getZipError()));
				outFileMap.write(image->data()) == image->data().size() OR_THROW(tr("outFileMap.write(): Błąd zapisu danych do archiwum"));
				outFileMap.close();
				outFileMap.getZipError() == UNZ_OK OR_THROW(tr("outFileMap.close(): %1").arg(outFileMap.getZipError()));
			}

			kmr.close();
			kmr.getZipError() == UNZ_OK OR_THROW(tr("kmr.close(): %1").arg(kmr.getZipError()));
		}
		jpg.open(QIODevice::Append);
		jpg.seek(jpg.size());
		QDataStream(&jpg) << thumbnailSize; // BigEndian by default
		jpg.close();

		QFile::remove(absoluteFilePath);
	}
}

Overlay::~Overlay()
{
	qDeleteAll(images);
}

QString Overlay::toString() const
{
	QStringList names;
	foreach (OverlayImage *image, images)
		names << image->name();

	if (names.size() == 1)
		return names.first();
	else
		return QString("%1 (%2)").arg(name).arg(names.join(", "));
}

bool Overlay::makeMap(GeoMap *map)
{
	foreach (OverlayImage *image, images)
		if (image->makeMap(map))
			return true;
	return false;
}

Overlay::DistancePair Overlay::bestDistance(GeoMap *map) const
{
	DistancePair best = qMakePair((OverlayImage *)nullptr, 10000.0);
	foreach (OverlayImage *image, images)
	{
		qreal current = image->distance(map);
		if (current < best.second)
			best = qMakePair(image, current);
	}
	return best;
}

void Overlay::writeThumbnail(QIODevice *device) const
{
	QPolygonF coordPolygon;
	foreach (OverlayImage *image, images)
		coordPolygon = coordPolygon.united(image->coordPolygon());
	const QRectF coordBox = coordPolygon.boundingRect();

//	qDebug() << name;

	const QString fontFamily = "Ubuntu";
	const QRect thumbnailRect(QPoint(), QSize(1400, 1050));
	const QRect partsRect = thumbnailRect.adjusted(10, 40, -10, -150);
	const QRegion region = QRegion(thumbnailRect).xored(QRegion(partsRect));
	const QRect header = region.rects().first().adjusted(10, 0, -10, 0);

	QPolygonF pointPolygon;
	QPolygonF boundingPoly;
	{
		const OverlayImage partsOverlay(coordBox, partsRect.size()); // for calculations only

		foreach (QPointF coord, coordPolygon)
			pointPolygon.append(partsOverlay.coordToPoint(coord));
		boundingPoly = minimumBoundingBox(pointPolygon.toList());
	}

	qreal angle;
	{
		const qreal l1 = QVector2D(boundingPoly[0] - boundingPoly[1]).length();
		const qreal l2 = QVector2D(boundingPoly[1] - boundingPoly[2]).length();
		const QLineF line(boundingPoly[1], boundingPoly[l1 > l2 ? 0 : 2]);
		angle = line.angle();
		while (angle > 90.0)
			angle -= 180.0;
	}

	qreal scaleFactor;
	QRectF partsRectScaled;
	{
		QRectF boundingRect = QMatrix().rotate(angle).map(boundingPoly).boundingRect();
		boundingRect.moveTopLeft(QPointF());

		QSizeF boundingSizeScaled = boundingRect.size();
		boundingSizeScaled.scale(partsRect.size(), Qt::KeepAspectRatio);
//		qDebug() << boundingRect.size() << boundingSizeScaled;

		scaleFactor = boundingSizeScaled.width() / boundingRect.width();
		partsRectScaled = QMatrix().scale(scaleFactor, scaleFactor).map(pointPolygon).boundingRect();
//		qDebug() << scaleFactor << maxRect;
	}

	const OverlayImage thumbnailOverlay(coordBox, partsRectScaled.size().toSize()); // for calculations only

	QImage parts(thumbnailOverlay.size(), QImage::Format_RGB32);
	parts.fill(Qt::black);
	{
		QPainter painter(&parts);
		painter.setRenderHints(QPainter::Antialiasing);
		painter.setPen(Qt::white);
		painter.setClipping(true);
		foreach (OverlayImage *image, images)
		{
			QPolygon pointPoly;
			foreach (QPointF coord, image->coordPolygon())
				pointPoly << thumbnailOverlay.coordToPoint(coord);

			QPainterPath path;
			path.addPolygon(pointPoly);
			painter.setClipPath(path);

			QImage part = image->render(pointPoly.boundingRect().size());
			painter.drawImage(pointPoly.boundingRect().topLeft(), part);
			painter.drawPolygon(pointPoly);
		}
	}

	QImage result(thumbnailRect.size(), QImage::Format_RGB32);
	result.fill(Qt::black);
	{
		QPainter painter(&result);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

		{
			const QRect partsClipRect = (QMatrix().scale(scaleFactor, scaleFactor) *
										 QImage::trueMatrix(QMatrix().rotate(angle), parts.width(), parts.height()))
										.map(boundingPoly).boundingRect().toRect();

			parts = parts.transformed(QMatrix().rotate(angle), Qt::SmoothTransformation).copy(partsClipRect);
		}

		QRect innerPartsRect = centered(partsRect.center(), parts.size());
		innerPartsRect.moveTop(partsRect.top());
		painter.drawImage(innerPartsRect, parts);

		painter.setPen(Qt::white);
		painter.setFont(QFont(fontFamily, 18));
		painter.drawText(header, name, Qt::AlignCenter | Qt::AlignVCenter);
		painter.setFont(QFont(fontFamily, 10));
		painter.drawText(header, coordsToString(coordBox.center()), Qt::AlignLeft | Qt::AlignVCenter);
		painter.drawText(header, QString("%1 - podkład mapowy").arg(qApp->applicationName()), Qt::AlignRight | Qt::AlignVCenter);

		{
			QStringList names;
			for (int i = 0; i < images.size(); ++i)
				names << ((images.size() > 1) ? QString("%1. ").arg(i + 1) : "") + QString("\"%1\"").arg(images[i]->name());
			QStringList techs;
			for (int i = 0; i < images.size(); ++i)
				techs << images[i]->tech();

			QRectF footer = QRect(QPoint(0, innerPartsRect.bottom()), thumbnailRect.bottomRight()).adjusted(10, 10, -10, -10);
			QRectF bounding;
			painter.setFont(QFont(fontFamily, 10));
			qreal bottom = 0;

			for (int i = 0; i < qCeil(names.size() / 10.0); ++i)
			{
				QStringList currentNames = names.mid(i * 10, 10);
				QStringList currentTechs = techs.mid(i * 10, 10);
				painter.drawText(footer.adjusted(bounding.right(), 0, 0, 0), Qt::AlignLeft | Qt::AlignTop, currentNames.join("\n"), &bounding);
				bottom = qMax(bottom, bounding.bottom());
				painter.drawText(footer.adjusted(bounding.right(), 0, 0, 0), Qt::AlignLeft | Qt::AlignTop, currentTechs.join("\n"), &bounding);
				bottom = qMax(bottom, bounding.bottom());
			}
			painter.drawText(footer.adjusted(bounding.right(), 0, 0, 0), Qt::AlignRight | Qt::AlignTop, description, &bounding);
			bottom = qMax(bottom, bounding.bottom());

			painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
			painter.drawRect(innerPartsRect);

			drawCompassRose(painter, QRectF(innerPartsRect.topLeft() + QPointF(10, 10), QSize(100, 100)), angle);
			painter.end();

			result = result.copy(QRect(thumbnailRect.topLeft(), QPoint(thumbnailRect.right(), bottom + 10)));
		}
	}

//	if (!device)
//	{
//		static int counter = 0;
//		result.save(QString("/home/kamil/Dokumenty/ssc-fotorelacje/overlays/thumbnail%1.jpeg").arg(++counter), "jpg", 90);
//		return;
//	}

	result.save(device, "jpg", 90);
	device->seek(0);

	ExifImageHeader exifHeader;
	exifHeader.setGpsPosition(coordBox.center());
	exifHeader.setValue(ExifImageHeader::Software, Application::applicationNameAndVersion());
	exifHeader.setValue(ExifImageHeader::ImageDescription, QString("%1, %2, %3, %4")
						.arg(coordBox.left(), 0, 'f', 8).arg(coordBox.top(), 0, 'f', 8)
						.arg(coordBox.right(), 0, 'f', 8).arg(coordBox.bottom(), 0, 'f', 8));
	exifHeader.setThumbnail(result.scaledToWidth(400, Qt::SmoothTransformation));
	exifHeader.saveToJpeg(device);
}

QString Overlay::coordsToString(QPointF coords)
{
	return QString("%1, %2").arg(coordToQString(coords.y(), "NS")).arg(coordToQString(coords.x(), "EW"));
}

QString Overlay::coordToQString(qreal coord, QString dir)
{
	const QChar c = coord >= 0 ? dir[0] : dir[1];
	const int d = coord;
	const int m = (coord -= d) *= 60;
	const qreal s = (coord -= m) *= 60;
	return QString("%1°%2′%3″%4").arg(d, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0')).arg(s, 5, 'f', 2, QChar('0')).arg(c);
}

inline bool operator < (const QPointF &p1, const QPointF &p2)
{
	return p1.x() < p2.x();
}

inline bool operator < (const QRectF &r1, const QRectF &r2)
{
	return r1.width() * r1.height() < r2.width() * r2.height();
}

QPolygonF Overlay::minimumBoundingBox(QList<QPointF> points) //, QPainter *painter)
{
	if (points.isEmpty())
		return QPolygonF();

	points = points.toSet().toList();
	qSort(points);

	QList<QPointF> left; // left (left top) contour
	{
		left << points.first();
		for (int i = 1; i < points.size(); ++i)
		{
			if (points[i].y() >= left.first().y())
				left.prepend(points[i]);
			else if (points[i].y() < left.last().y())
				left.append(points[i]);
		}
	}

	QList<QPointF> right; // right (right bottom) contour
	{
		right << points.last();
		for (int i = points.size() - 2; i >= 0; --i)
		{
			if (points[i].y() <= right.first().y())
				right.prepend(points[i]);
			else if (points[i].y() > right.last().y())
				right.append(points[i]);
		}
	}

	Q_ASSERT(left.last() == right.first() && left.first() == right.last());

	left.takeLast();

	QPolygonF convexPolygon = (left + right).toVector();
//	qDebug();
//	qDebug() << convexPolygon;

	// make polygon convex by removing points concave points
	int oldSize;
	do
	{
		oldSize = convexPolygon.size();
		for (int i = 0; i < convexPolygon.size();)
		{
			QPolygonF testPolygon = convexPolygon;
			testPolygon.remove(i);
			if (testPolygon.containsPoint(convexPolygon[i], Qt::OddEvenFill))
				convexPolygon = testPolygon;
			else
				++i;
		}
	}
	while (convexPolygon.size() != oldSize);

//	qDebug() << convexPolygon;

	// iterate over convex polygon and find MBR
	QMap<QRectF, QPolygonF> rectangles;
	for (int i = 0; i < convexPolygon.size() - 1; ++i)
	{
		const QLineF line(convexPolygon[i], convexPolygon[i + 1]);
		const QRectF rect = QMatrix().rotate(line.angle()).map(convexPolygon).boundingRect();
		const QPolygonF polygon = QMatrix().rotate(-line.angle()).map(QPolygonF(rect));
		rectangles[rect] = polygon;
	}
	return rectangles.begin().value();
}

void Overlay::drawCompassRose(QPainter &painter, QRectF rect, qreal angle)
{
	painter.setWorldMatrix(QMatrix().translate(rect.center().x(), rect.center().y()).rotate(angle).scale(rect.width() / 2, rect.height() / 2));
	painter.setWorldMatrixEnabled(true);
	painter.setPen(QPen(Qt::gray, 0.015, Qt::SolidLine));

	QPolygonF right, left;
	right << QPointF() << QPointF(0.15, -0.15) << QPointF(0, -1) << QPointF();
	left << QPointF() << QPointF(-0.15, -0.15) << QPointF(0, -1) << QPointF();

	qreal scale = 0.6;
	painter.setWorldMatrix(QMatrix(painter.worldMatrix()).rotate(-45).scale(scale, scale));

	for (int j = 0; j < 2; ++j)
	{
		for (int i = 0; i < 4; ++i)
		{
			painter.setBrush(Qt::black);
			painter.drawPolygon(right);
			painter.setBrush(i == 0 && j == 1 ? Qt::red : Qt::white);
			painter.drawPolygon(left);
			painter.setWorldMatrix(QMatrix(painter.worldMatrix()).rotate(90));
		}
		painter.setWorldMatrix(QMatrix(painter.worldMatrix()).rotate(45).scale(1 / scale, 1 / scale));
	}
	painter.setWorldMatrixEnabled(false);
}
