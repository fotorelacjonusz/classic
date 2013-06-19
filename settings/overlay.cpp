#include "overlay.h"

#include "quazip/quazip.h"
#include "quazip/quazipfile.h"

#include <QDomDocument>
#include <QDomNode>
#include <QMap>
#include <QDebug>
#include <QFile>

#define DOC_KML "doc.kml"
//#define TR(x) QObject::tr(x)
//#define ERROR(msg) setError(msg); if (!error.isEmpty()) return;

Overlay::Overlay(QString absoluteFilePath) throw (Exception)
{
	QMap<QString, QByteArray> files;
	{
		QuaZip kmz(absoluteFilePath);
		kmz.open(QuaZip::mdUnzip) OR_THROW(TR("kmz.open(): %1").arg(kmz.getZipError()));

		QuaZipFile file(&kmz);
		for (bool f = kmz.goToFirstFile(); f; f = kmz.goToNextFile())
		{
			file.open(QIODevice::ReadOnly) OR_THROW(TR("file.open(): %1").arg(file.getZipError()));
			files[file.getActualFileName()] = file.readAll();
			file.close();
			file.getZipError() == UNZ_OK OR_THROW(TR("file.close(): %1").arg(file.getZipError()));
		}

		kmz.close();
		kmz.getZipError() == UNZ_OK OR_THROW(TR("kmz.close(): %1").arg(kmz.getZipError()));
	}

//	qDebug() << files.keys();
	files.contains(DOC_KML) OR_THROW(TR("Brak pliku '%1'").arg(DOC_KML));
	bool isKmz = absoluteFilePath.endsWith(".kmz");
	
	QDomDocument doc;
	doc.setContent(files[DOC_KML]) OR_THROW(TR("Niepoprawna składnia kml"));
	QDomElement kml = doc.documentElement();
	QDomElement firstChild = kml.firstChild().toElement();
	(!kml.isNull() && !firstChild.isNull()) OR_THROW(TR("Brak tagu kml i/lub podrzędnego w pliku kml"));
	
	name = firstChild.firstChildElement("name").text();
	
	if (firstChild.tagName() == "GroundOverlay")
		images << new OverlayImage(firstChild, files, isKmz);
	else if (firstChild.tagName() == "Folder")
	{
		QDomNodeList overlays = firstChild.elementsByTagName("GroundOverlay");
		for (int i = 0; i < overlays.size(); ++i)
			images << new OverlayImage(overlays.at(i).toElement(), files, isKmz);
	}
	else
		THROW(TR("Brak tagu Folder lub GroundOverlay"));
	
	if (isKmz)
	{
		QString kmrFilePath = absoluteFilePath.section(".", 0, -2) + ".kmr";
		QuaZip kmr(kmrFilePath);
		kmr.open(QuaZip::mdCreate) OR_THROW(TR("kmr.open(): %1").arg(kmr.getZipError()));

		QuaZipFile outFileKml(&kmr);
		outFileKml.open(QIODevice::WriteOnly, QuaZipNewInfo(DOC_KML, absoluteFilePath)) OR_THROW(TR("outFileKml.open(): %1").arg(outFileKml.getZipError()));
		outFileKml.write(doc.toByteArray());
		outFileKml.close();
		outFileKml.getZipError() == UNZ_OK OR_THROW(TR("outFileKml.close(): %1").arg(outFileKml.getZipError()));

		foreach (OverlayImage *image, images)
		{
			QuaZipFile outFileMap(&kmr);
			outFileMap.open(QIODevice::WriteOnly, QuaZipNewInfo(image->href(), absoluteFilePath)) OR_THROW(TR("outFileMap.open(): %1").arg(outFileMap.getZipError()));
			outFileMap.write(image->data()) == image->data().size() OR_THROW(TR("outFileMap.write(): Błąd zapisu danych do archiwum"));
			outFileMap.close();
			outFileMap.getZipError() == UNZ_OK OR_THROW(TR("outFileMap.close(): %1").arg(outFileMap.getZipError()));
		}

		kmr.close();
		kmr.getZipError() == UNZ_OK OR_THROW(TR("kmr.close(): %1").arg(kmr.getZipError()));
		
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
		names << image->toString();
	
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
	DistancePair best = qMakePair((OverlayImage *)0, 10000.0);
	foreach (OverlayImage *image, images)
	{
		qreal current = image->distance(map);
		if (current < best.second)
			best = qMakePair(image, current);
	}
	return best;
}
