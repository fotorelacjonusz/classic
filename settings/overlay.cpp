#include "overlay.h"
#include "settingsdialog.h"

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

	poly << rotate(orthoProjection(left, top)) << rotate(orthoProjection(right, top))
		 << rotate(orthoProjection(right, bottom)) << rotate(orthoProjection(left, bottom));
	box = poly.boundingRect();

//	qDebug() << box;

//	box.isValid() or ERROR(TR("Niepoprawne koordynaty"));
	files.contains(href) or ERROR(TR("Brak pliku %1 w archiwum kmz"));
	map.loadFromData(files[href]) or ERROR(TR("Nieudane ładowanie pliku z mapą"));
	if (absoluteFilePath.endsWith(".kmr"))
		return;

	QTransform transform;
	transform.rotate(-rotation);
	map = map.transformed(transform, Qt::SmoothTransformation);
	{
		QString kmrFilePath = absoluteFilePath.section(".", 0, -2) + ".kmr";
		QuaZip kmr(kmrFilePath);
		kmr.open(QuaZip::mdCreate) or ERROR(TR("kmr.open(): %1").arg(kmr.getZipError()));

		QuaZipFile outFileKml(&kmr);
		outFileKml.open(QIODevice::WriteOnly, QuaZipNewInfo(DOC_KML)) or ERROR(TR("outFileKml.open(): %1").arg(outFileKml.getZipError()));
		outFileKml.write(doc.toByteArray());
		outFileKml.close();
		outFileKml.getZipError() == UNZ_OK or ERROR(TR("outFileKml.close(): %1").arg(outFileKml.getZipError()));

		QuaZipFile outFileMap(&kmr);
		outFileMap.open(QIODevice::WriteOnly, QuaZipNewInfo(href)) or ERROR(TR("outFileMap.open(): %1").arg(outFileMap.getZipError()));
		map.save(&outFileMap, "jpg") or ERROR(TR("map.save(): Błąd zapisu pixmapy do archiwum"));
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

bool Overlay::contains(qreal lon, qreal lat) const
{
	return isValid() && poly.containsPoint(orthoProjection(lon, lat), Qt::OddEvenFill);
}

QPixmap Overlay::makeMap(qreal lon, qreal lat) const
{
	QPointF pos = orthoProjection(lon, lat);
	pos.setY(pos.y() * -1);
	if (!box.contains(pos))
		return QPixmap();

	pos -= box.topLeft();
	pos.setX(pos.x() * map.width() / box.width());
	pos.setY(pos.y() * map.height() / box.height());

	int zoom = 22 - SETTINGS->imageMapZoom; // 1 - 22
	int size = zoom * SETTINGS->imageMapSize;

	QRect copyRect(pos.x() - size / 2, pos.y() - size / 2, size, size);
	QPixmap mapCopy(size, size);

	QPainter painter(&mapCopy);
	painter.fillRect(mapCopy.rect(), QColor(Qt::black));
	painter.drawPixmap(QPoint(copyRect.left() < 0 ? size - mapCopy.width()  : 0,
							  copyRect.top()  < 0 ? size - mapCopy.height() : 0), map, copyRect);
	painter.end();

	return mapCopy.scaledToWidth(SETTINGS->imageMapSize, Qt::SmoothTransformation);
}

QPointF Overlay::rotate(QPointF point) const
{
	qreal alpha = atan2(point.y(), point.x());
	qreal r = sqrt(point.x() * point.x() + point.y() * point.y());
	qreal beta = alpha + rotation * M_PI / 180;
	return QPointF(cos(beta) * r, sin(beta) * r);
}

QPointF Overlay::orthoProjection(qreal lon, qreal lat) const
{
	lon *= M_PI / 180;
	lat *= M_PI / 180;
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



