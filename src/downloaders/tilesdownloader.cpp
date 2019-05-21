#include "tilesdownloader.h"
#include "throttlednetworkmanager.h"

#include <QRect>
#include <QPainter>
#include <QDebug>
#include <QNetworkReply>
#include <QMessageBox>
#include <QApplication>
#include <qmath.h>

#include "settings/settingsdialog.h"
#include "gpsdata.h"
#include "geomap.h"

const QStringList TilesDownloader::varNames = QStringList() << "${z}" << "${x}" << "${y}";
const int TilesDownloader::tileSize = 256;

TilesDownloader::TilesDownloader(QString urlPattern):
	urlPattern(urlPattern), painter(nullptr)
{
	connect(this, SIGNAL(makeMapSignal(GeoMap*)), this, SLOT(makeMapSlot(GeoMap*)));
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

void TilesDownloader::makeMapSlot(GeoMap *map)
{
	urlPattern = SETTINGS->currentOsmLayer(map->isCommon).urlPattern;

	if (!map->isCommon)
		map->setImage(render(map->first(), SETTINGS->imageMapZoom, map->size));
	else if (map->isSingle)
		map->setImage(render(map->first(), 17, maxSize()), QSize(0, 0));
	else
	{
		// calculate maximum integer zoom for which width and height are less than their maximums including margins
		const int zx = qFloor(log2((maxSize().width()  - 2 * margin) / (tileSize * qAbs(lon2tilex(map->coordBox.right())  - lon2tilex(map->coordBox.left())))));
		const int zy = qFloor(log2((maxSize().height() - 2 * margin) / (tileSize * qAbs(lat2tiley(map->coordBox.bottom()) - lat2tiley(map->coordBox.top())))));
		const int zoom = qMin(qMin(zx, zy), SETTINGS->currentOsmLayer(true).maxZoom);
		const int width =  qAbs(lon2tilex(map->coordBox.right(),  zoom) - lon2tilex(map->coordBox.left(), zoom)) * tileSize;
		const int height = qAbs(lat2tiley(map->coordBox.bottom(), zoom) - lat2tiley(map->coordBox.top(),  zoom)) * tileSize;
		map->setImage(render(map->coordBox.center(), zoom, maxSize()), QSize(width, height));
	}
}

QImage TilesDownloader::render(QPointF coord, int zoom, QSize size)
{
	QPoint tileTopLeft;
	QSize tileCount;
	QPointF cropBegin;

	// calculate which tiles we need and how to crop them later
	calculateDimension(lon2tilex(coord.x(), zoom), size.width()  * 0.5, tileTopLeft.rx(), tileCount.rwidth(),  cropBegin.rx());
	calculateDimension(lat2tiley(coord.y(), zoom), size.height() * 0.5, tileTopLeft.ry(), tileCount.rheight(), cropBegin.ry());

	QRect tileRect(tileTopLeft, tileCount);
	QImage image(tileCount * tileSize, QImage::Format_RGB32);
	painter = new QPainter(&image);
	painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

	// enqueue all tiles and start two
	static ThrottledNetworkManager manager(2);
	downloadCount = tileCount.width() * tileCount.height();
	for (int x = tileRect.left(); x <= tileRect.right(); ++x)
		for (int y = tileRect.top(); y <= tileRect.bottom(); ++y)
			manager.get(createRequest(zoom, x, y, tileTopLeft), this);

	// wait for all tiles being downloaded and painted with painter
	loop.exec();

	delete painter;
	painter = nullptr;

	// crop
	QRect cropRect(cropBegin.toPoint(), size);
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
