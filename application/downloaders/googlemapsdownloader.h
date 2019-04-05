#ifndef GOOGLEMAPSDOWNLOADER_H
#define GOOGLEMAPSDOWNLOADER_H

#include <QObject>
#include <QPixmap>
#include <QUrl>
#include <QCoreApplication>

#include "abstractmapdownloader.h"

class GoogleMapsDownloader : public AbstractMapDownloader
{
	Q_DECLARE_TR_FUNCTIONS(GoogleMapsDownloader)

public:
	bool makeMap(GeoMap *map);
	void finished(QNetworkReply *reply);

protected:
	QHash<QUrl, GeoMap *> maps;
};

#endif // GOOGLEMAPSDOWNLOADER_H
