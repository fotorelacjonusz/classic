#include "abstractmapdownloader.h"
#include "gpsdata.h"
#include "geomap.h"

#include <QMetaType>

AbstractMapDownloader::AbstractMapDownloader()
{
	static bool registered = false;
	if (!registered)
	{
		qRegisterMetaType<GeoMap *>("GeoMap*");
		registered = true;
	}
}

AbstractMapDownloader::~AbstractMapDownloader()
{
}

void AbstractMapDownloader::finished(QNetworkReply *reply)
{
	Q_UNUSED(reply);
}

QRect centered(const QPoint &center, const QSize &size)
{
	QRect rect(center, size);
	rect.moveBottomRight(rect.center() + QPoint(1, 1));
	return rect;
}

QRect centered(const QPoint &center, const int size)
{
	return centered(center, QSize(size, size));
}

QRect resized(const QRect &rect, const qreal factor)
{
	return centered(rect.center(), rect.size() * factor);
}

QRect resized(const QRect &rect, QSize size)
{
	return centered(rect.center(), size);
}
