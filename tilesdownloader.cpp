#include "tilesdownloader.h"
#include "throttlednetworkmanager.h"

#include <QRect>
#include <QPainter>
#include <QDebug>

#include <QNetworkReply>
#include <QMessageBox>
#include <QApplication>
#include <qmath.h>

#include "settingsdialog.h"
#include "gpsdata.h"

const QStringList TilesDownloader::varNames = QStringList() << "${z}" << "${x}" << "${y}";
const int TilesDownloader::tileSize = 256;

TilesDownloader::TilesDownloader(QString urlPattern):
	urlPattern(urlPattern), painter(0), //format(urlPattern.section('.', -1).toAscii()),
	balloon(":/res/balloon.15-1-8.8-2.12.png")
{
}

bool TilesDownloader::validateUrlPattern(QWidget *parent, QString urlPattern)
{
	foreach (QString var, varNames)
	{
		if (!urlPattern.contains(var))
		{
			QMessageBox::critical(parent, tr("Błąd"), tr("Podany adres nie zawiera zmiennej %1").arg(var));
			return false;
		}
	}
	if (!urlPattern.endsWith(".png"))
	{
		QMessageBox::critical(parent, tr("Błąd"), tr("Podany adres nie kończy się na .png"));
		return false;
	}
	return true;
}

void TilesDownloader::downloadMap(QPointF point)
{
	urlPattern = SETTINGS->currentOsmUrlPattern(false);
	QMetaObject::invokeMethod(this, "render", Qt::QueuedConnection, Q_ARG(QPointF, point));
}

void TilesDownloader::downloadMap(Points points)
{
	urlPattern = SETTINGS->currentOsmUrlPattern(true);
	QMetaObject::invokeMethod(this, "render", Qt::QueuedConnection, Q_ARG(Points, points));
}

void TilesDownloader::render(QPointF point)
{
	emit mapDownloaded(render(point.x(), point.y(), SETTINGS->imageMapZoom, SETTINGS->imageMapSize, SETTINGS->imageMapSize));
}

void TilesDownloader::render(Points points)
{
	static const int maxWidth = 800, maxHeight = 600, margin = 50;
	
	QList<QPointF> distinctPoints = points.values().toSet().toList();
	
	if (distinctPoints.isEmpty())
		return;
	else if (distinctPoints.size() == 1)
	{
		// render map with maximum size with default zoom
		QImage image = render(distinctPoints.first().x(), distinctPoints.first().y(), 15, maxWidth, maxHeight);
		// draw one marker in center
		QPainter painter(&image);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		QImage marker = balloon.render(*points.begin().key()->number + SETTINGS->startingNumber);
		painter.drawImage(image.rect().center().x(), image.rect().center().y() - marker.height(), marker);
		emit mapDownloaded(image);
		return;
	}
	else // if (distinctPoints.size() > 1)
	{
		// make box containing all points
		QRectF box = QRectF(distinctPoints.takeFirst(), distinctPoints.takeLast()).normalized();
		foreach (QPointF point, distinctPoints)
			box |= QRectF(point, box.center()).normalized();
		
		// calculate maximum integer zoom for which width and height are less than their maximums
		int zx = qFloor(log2(maxWidth  / (tileSize * qAbs(lon2tilex(box.right())  - lon2tilex(box.left())))));
		int zy = qFloor(log2(maxHeight / (tileSize * qAbs(lat2tiley(box.bottom()) - lat2tiley(box.top())))));
		int zoom = qMin(zx, zy);
		int width =  qAbs(lon2tilex(box.right(),  zoom) - lon2tilex(box.left(), zoom)) * tileSize;
		int height = qAbs(lat2tiley(box.bottom(), zoom) - lat2tiley(box.top(),  zoom)) * tileSize;
//		qDebug() << zx << zy << zoom << width << height;
		
		QImage image = render(box.center().x(), box.center().y(), zoom, width + margin * 2, height + margin * 2);
		{
			// draw markers fo all points
			QPainter painter(&image);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
			
			for (Points::Iterator i = points.begin(); i != points.end(); ++i)
//			for (int i = 0; i < points.size(); ++i)
			{
				int x = margin + (i.value().x() - box.left()) / box.width() * width;
				int y = margin + (box.bottom() - i.value().y()) / box.height() * height;
//				qDebug() << *i.key()->number << x << y;
				QImage marker = balloon.render(*i.key()->number + SETTINGS->startingNumber);
				painter.drawImage(x, y - marker.height(), marker);
			}
		}	
		emit mapDownloaded(image);
		return;
	}
}

QImage TilesDownloader::render(qreal lon, qreal lat, int zoom, int width, int height)
{
	QPoint begin; 
	QSize size;
	QPointF cropBegin;

	// calculate which tiles we need and how to crop them later
	calculateDimension(lon2tilex(lon, zoom), width / 2.0,  begin.rx(), size.rwidth(),  cropBegin.rx());
	calculateDimension(lat2tiley(lat, zoom), height / 2.0, begin.ry(), size.rheight(), cropBegin.ry());
	
	QRect tileRect(begin, size);
//	qDebug() << tileRect;
	QImage image(size * tileSize, QImage::Format_RGB32);
	painter = new QPainter(&image);
	painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
	
	// enqueue all tiles and start two
	static ThrottledNetworkManager manager(2);
	downloadCount = size.width() * size.height();
	for (int x = tileRect.left(); x <= tileRect.right(); ++x)
		for (int y = tileRect.top(); y <= tileRect.bottom(); ++y)
			manager.get(createRequest(zoom, x, y, begin), this);
	
	// wait for all tiles
	loop.exec();
	
	painter->end();
	delete painter;
	painter = 0;
	
	QRect cropRect(cropBegin.toPoint(), QSize(width, height));
//	qDebug() << cropRect;
	return image.copy(cropRect);
}

qreal TilesDownloader::lon2tilex(qreal lon)
{
	return (lon + 180.0) / 360.0;
}

qreal TilesDownloader::lon2tilex(qreal lon, int z) 
{ 
	return lon2tilex(lon) * (1 << z); 
}

qreal TilesDownloader::lat2tiley(qreal lat)
{
	return (1.0 - qLn(qTan(lat * M_PI / 180.0) + 1.0 / qCos(lat * M_PI / 180.0)) / M_PI) / 2.0;
}
 
qreal TilesDownloader::lat2tiley(qreal lat, int z)
{ 
	 return lat2tiley(lat) * (1 << z);
}

void TilesDownloader::calculateDimension(qreal value, qreal halfDimension, int &begin, int &size, qreal &cropBegin)
{
	int vf = qFloor(value);
	qreal vr = value - vf;
	int v1 = qCeil(halfDimension / tileSize - vr);
	int v2 = qCeil(halfDimension / tileSize + vr);
	begin = vf - v1;
	size = v1 + v2;
	cropBegin = (value - begin) * tileSize - halfDimension;
}

QNetworkRequest TilesDownloader::createRequest(int zoom, int x, int y, QPoint begin)
{
	QString url = urlPattern;
	QList<int> vars = QList<int>() << zoom << x << y;
	for (int i = 0; i < vars.size(); ++i)
		url.replace(varNames[i], QString::number(vars[i]));

	QNetworkRequest request(url);
	request.setRawHeader("User-Agent", (qApp->applicationName() + " " + qApp->applicationVersion()).toAscii());
	urlToTilePos[request.url()] = (QPoint(x, y) - begin) * tileSize;
	return request;
}

void TilesDownloader::finished(QNetworkReply *reply)
{
	if (reply->error() != QNetworkReply::NoError)
	{
//		qDebug() << tr("Błąd pobierania mapy: %1 %2").arg(reply->error()).arg(reply->errorString());
		QRect rect(urlToTilePos[reply->url()], QSize(tileSize, tileSize));
		painter->drawText(rect, Qt::TextWordWrap, tr("Błąd pobierania mapy: %1 %2").arg(reply->error()).arg(reply->errorString()));
		painter->drawRect(rect);
	}
	else
	{
		QImage tile;
		tile.loadFromData(reply->readAll());
		painter->drawImage(urlToTilePos[reply->url()], tile);
	}
	
	if (--downloadCount == 0)
		loop.exit();
	reply->deleteLater();
}


