#ifndef GOOGLEMAPSDOWNLOADER_H
#define GOOGLEMAPSDOWNLOADER_H

#include <QObject>
#include <QPixmap>
#include <QUrl>

#include "abstractmapdownloader.h"

class GoogleMapsDownloader : public AbstractMapDownloader
{
public:
	bool makeMap(GeoMap *map);
	void finished(QNetworkReply *reply);

protected:
	QHash<QUrl, GeoMap *> maps;
};

#endif // GOOGLEMAPSDOWNLOADER_H
