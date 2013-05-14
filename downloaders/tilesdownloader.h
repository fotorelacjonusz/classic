#ifndef TILESDOWNLOADER_H
#define TILESDOWNLOADER_H

#include "abstractmapdownloader.h"

#include <QNetworkRequest>
#include <QEventLoop>
#include <QHash>
#include <QImage>

class QPainter;

class TilesDownloader : public QObject, public AbstractMapDownloader
{
	Q_OBJECT
	
public:
	TilesDownloader(QString urlPattern = QString());
	static bool validateUrlPattern(QWidget *parent, QString urlPattern);
	bool makeMap(GeoMap *map);
	
protected slots:
	void render(GeoMap *map);
	QImage render(QPointF coord, int zoom, int width, int height);
	
protected:
	static inline qreal lon2tilex(qreal lon);
	static inline qreal lon2tilex(qreal lon, int z);
	static inline qreal lat2tiley(qreal lat);
	static inline qreal lat2tiley(qreal lat, int z);
	static void calculateDimension(qreal value, qreal halfDimension, int &begin, int &size, qreal &cropBegin);
	
	QNetworkRequest createRequest(int zoom, int x, int y, QPoint begin);
	void finished(QNetworkReply *reply);
	
private:	
	QString urlPattern;
	
	QPainter *painter;
	int downloadCount;
	QHash<QUrl, QPoint> urlToTilePos;
	QEventLoop loop;
		
	static const int tileSize;
	static const QStringList varNames;
};

#endif // TILESDOWNLOADER_H
