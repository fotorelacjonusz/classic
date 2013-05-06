#ifndef TILESDOWNLOADER_H
#define TILESDOWNLOADER_H

#include "textballoon.h"
#include "throttlednetworkmanager.h"
#include "abstractmapdownloader.h"

#include <QPoint>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QQueue>
#include <QHash>

class QPainter;

class TilesDownloader : public AbstractMapDownloader
{
	Q_OBJECT
	
public:
	TilesDownloader(QString urlPattern = QString());
	static bool validateUrlPattern(QWidget *parent, QString urlPattern);
	
	void downloadMap(QPointF point);
	void downloadMap(Points points);
	
protected slots:
	void render(QPointF point);
	void render(Points points);
	QImage render(qreal lon, qreal lat, int zoom, int width, int height);
	
protected:
	static inline qreal lon2tilex(qreal lon);
	static inline qreal lon2tilex(qreal lon, int z);
	static inline qreal lat2tiley(qreal lat);
	static inline qreal lat2tiley(qreal lat, int z);
	static void calculateDimension(qreal value, qreal halfDimension, int &begin, int &size, qreal &cropBegin);
	
	QNetworkRequest createRequest(int zoom, int x, int y, QPoint begin);
	
protected slots:
	void finished(QNetworkReply *reply);
	
private:	
	QString urlPattern;
	
	QPainter *painter;
	int downloadCount;
	QHash<QUrl, QPoint> urlToTilePos;
	QEventLoop loop;
		
	static const int tileSize;
	static const QStringList varNames;
	
	TextBalloon balloon;
};

#endif // TILESDOWNLOADER_H
