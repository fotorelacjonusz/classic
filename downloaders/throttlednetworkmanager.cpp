#include "throttlednetworkmanager.h"
#include "abstractmapdownloader.h"

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QDebug>

class Wrapper : public QObject
{
	Q_OBJECT
public:
	AbstractMapDownloader * const downloader;
	Wrapper(AbstractMapDownloader *downloader):
		downloader(downloader)
	{}
};
#include "throttlednetworkmanager.moc"

ThrottledNetworkManager::ThrottledNetworkManager(int limit, QObject *parent) :
	QObject(parent), limit(limit), running(0)
{
	cache.setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
	manager.setCache(&cache);
	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

void ThrottledNetworkManager::get(QNetworkRequest request, AbstractMapDownloader *downloader)
{
	request.setOriginatingObject(new Wrapper(downloader));
	requests.enqueue(request);
	start();
}

void ThrottledNetworkManager::finished(QNetworkReply *reply)
{
	--running;
	Wrapper *wrapper = qobject_cast<Wrapper *>(reply->request().originatingObject());
	Q_ASSERT(wrapper);
	wrapper->downloader->finished(reply);
	wrapper->deleteLater();
	reply->deleteLater();
	start();
}

void ThrottledNetworkManager::start()
{
	for (; running < limit && !requests.isEmpty(); ++running)
		manager.get(requests.dequeue());
}
