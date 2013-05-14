#include "overlay.h"
#include "settingsdialog.h"
#include "downloaders/geomap.h"

#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

#include <QDomDocument>
#include <QDomNode>
#include <QMap>
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <math.h>

#define DOC_KML "doc.kml"
#define TR(x) QObject::tr(x)
#define ERROR(msg) setError(msg); if (!error.isEmpty()) return;

const qreal Overlay::R = 6371000; // radius of the earth [m]

Overlay::Overlay(QString absoluteFilePath)
{
	QMap<QString, QByteArray> files;
	{
		QuaZip kmz(absoluteFilePath);
		kmz.open(QuaZip::mdUnzip) or ERROR(TR("kmz.open(): %1").arg(kmz.getZipError()));

		QuaZipFile file(&kmz);
		for (bool f = kmz.goToFirstFile(); f; f = kmz.goToNextFile())
		{
			file.open(QIODevice::ReadOnly) or ERROR(TR("file.open(): %1").arg(file.getZipError()));
			files[file.getActualFileName()] = file.readAll();
			file.close();
			file.getZipError() == UNZ_OK or ERROR(TR("file.close(): %1").arg(file.getZipError()));
		}

		kmz.close();
		kmz.getZipError() == UNZ_OK or ERROR(TR("kmz.close(): %1").arg(kmz.getZipError()));
	}

	files.contains(DOC_KML) or ERROR(TR("Brak pliku %1").arg(DOC_KML));

	QDomDocument doc;
	doc.setContent(files[DOC_KML]) or ERROR(TR("Niepoprawna składnia kml"));
	QDomElement kml = doc.documentElement();
	QDomElement groundOverlay = kml.firstChildElement("GroundOverlay");
	QDomElement icon = groundOverlay.firstChildElement("Icon");
	QDomElement latLonBox = groundOverlay.firstChildElement("LatLonBox");

	(!kml.isNull() && !groundOverlay.isNull() && !icon.isNull()) or ERROR(TR("Brak tagu w pliku kml"));
	name = groundOverlay.firstChildElement("name").text();
	href = icon.firstChildElement("href").text();

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

//	box.isValid() or ERROR(TR("Niepoprawne koordynaty"));
	files.contains(href) or ERROR(TR("Brak pliku %1 w archiwum kmz"));
	overlayImage.loadFromData(files[href]) or ERROR(TR("Nieudane ładowanie pliku z mapą"));
	if (absoluteFilePath.endsWith(".kmr"))
		return;

	QTransform transform;
	transform.rotate(-rotation);
	overlayImage = overlayImage.transformed(transform, Qt::SmoothTransformation);
	{
		QString kmrFilePath = absoluteFilePath.section(".", 0, -2) + ".kmr";
		QuaZip kmr(kmrFilePath);
		kmr.open(QuaZip::mdCreate) or ERROR(TR("kmr.open(): %1").arg(kmr.getZipError()));

		QuaZipFile outFileKml(&kmr);
		outFileKml.open(QIODevice::WriteOnly, QuaZipNewInfo(DOC_KML, absoluteFilePath)) or ERROR(TR("outFileKml.open(): %1").arg(outFileKml.getZipError()));
		outFileKml.write(doc.toByteArray());
		outFileKml.close();
		outFileKml.getZipError() == UNZ_OK or ERROR(TR("outFileKml.close(): %1").arg(outFileKml.getZipError()));

		QuaZipFile outFileMap(&kmr);
		outFileMap.open(QIODevice::WriteOnly, QuaZipNewInfo(href, absoluteFilePath)) or ERROR(TR("outFileMap.open(): %1").arg(outFileMap.getZipError()));
		overlayImage.save(&outFileMap, "jpg") or ERROR(TR("map.save(): Błąd zapisu pixmapy do archiwum"));
		outFileMap.close();
		outFileMap.getZipError() == UNZ_OK or ERROR(TR("outFileMap.close(): %1").arg(outFileMap.getZipError()));

		kmr.close();
		kmr.getZipError() == UNZ_OK or ERROR(TR("kmr.close(): %1").arg(kmr.getZipError()));
	}
	QFile::remove(absoluteFilePath);
}

bool Overlay::isValid() const
{
	return error.isEmpty();
}

bool Overlay::makeMap(GeoMap *map)
{
	if (!isValid())
		return false;
	foreach (QPointF coord, map->distinctCoords)
		if (!poly.containsPoint(orthoProjection(coord), Qt::OddEvenFill))
			return false;
	
	if (map->isCommon)
	{
		// TODO: common map
	}
	else
	{
		QPointF pos = orthoProjection(map->coords.first());
		pos.setY(pos.y() * -1);
		if (!box.contains(pos))
			return false;
	
		pos -= box.topLeft();
		pos.rx() *= overlayImage.width() / box.width();
		pos.ry() *= overlayImage.height() / box.height();
	
		QRect copyRect = centered(pos.toPoint(), map->size * (22 - SETTINGS->imageMapZoom));
		QImage mapCopy(copyRect.size(), QImage::Format_ARGB32);
		mapCopy.fill(Qt::black);
		QPainter(&mapCopy).drawImage(QPoint(0, 0), overlayImage, copyRect);
			
		map->setImage(mapCopy.scaled(map->size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
	
	return true;
}

QString Overlay::toString() const
{
	return name + "\t" + error;
}

QPointF Overlay::rotate(QPointF point) const
{
	qreal alpha = atan2(point.y(), point.x());
	qreal r = sqrt(point.x() * point.x() + point.y() * point.y());
	qreal beta = alpha + rotation * M_PI / 180;
	return QPointF(cos(beta) * r, sin(beta) * r);
}

QPointF Overlay::orthoProjection(QPointF coords) const
{
	qreal lon = coords.x() * M_PI / 180;
	qreal lat = coords.y() * M_PI / 180;
	qreal x = R * cos(lat) * sin(lon - lon0);
	qreal y = R * (cos(lat0) * sin(lat) - sin(lat0) * cos(lat) * cos(lon - lon0));
	return QPointF(x, y);
}

QPointF Overlay::inverseOrthoProjection(QPointF point) const
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

bool Overlay::setError(QString error)
{
	this->error = error;
	return true;
}



