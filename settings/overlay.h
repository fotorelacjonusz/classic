#ifndef OVERLAY_H
#define OVERLAY_H

#include "overlayimage.h"
#include "exception.h"
#include <QString>

class Overlay : public AbstractMapDownloader
{
public:
	Overlay(QString absoluteFilePath) throw (Exception);
	~Overlay();

	QString toString() const;
	bool makeMap(GeoMap *map);
	
private:
	QString name;
	QList<OverlayImage *> images;
};

#endif // OVERLAY_H
