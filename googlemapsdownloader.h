#ifndef GOOGLEMAPSDOWNLOADER_H
#define GOOGLEMAPSDOWNLOADER_H

#include <QObject>
#include <QPixmap>
#include <QUrl>

#include "abstractmapdownloader.h"

class GoogleMapsDownloader : public AbstractMapDownloader
{
	Q_OBJECT
	
public:
	GoogleMapsDownloader();
	
	void downloadMap(QPointF point);
	void downloadMap(Points points);
	void finished(QNetworkReply *reply);
	
protected:
	void get(QUrl url);
};

#endif // GOOGLEMAPSDOWNLOADER_H
