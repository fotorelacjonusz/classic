#include "throttlednetworkmanager.h"
#include "tilesdownloader.h"
#include "abstractmapdownloader.h"

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QDebug>

ThrottledNetworkManager::ThrottledNetworkManager(int limit, QObject *parent) :
	QObject(parent),
	limit(limit),
	running(0)	
{
	cache.setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
	manager.setCache(&cache);
	connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

void ThrottledNetworkManager::get(QNetworkRequest request, AbstractMapDownloader *downloader)
{
	requests.append(qMakePair(request, downloader));
	start();
}

void ThrottledNetworkManager::finished(QNetworkReply *reply)
{
	--running;
	listeners[reply]->finished(reply);
	listeners.remove(reply);
	reply->deleteLater();
//	qDebug() << "finished" << running;
	start();
}

//	requestQueue.enqueue(qMakePair(request, downloader));
//	start();

void ThrottledNetworkManager::start()
{
	while (running < limit && !requests.isEmpty())
	{
		QPair<QNetworkRequest, AbstractMapDownloader *> pair = requests.dequeue();
		listeners.insert(manager.get(pair.first), pair.second);
				
//		QNetworkReply *reply = pair.second->get(pair.first);
//		connect(reply, SIGNAL(finished()), this, SLOT(finished()));
		++running;
//		qDebug() << "started" << running;
	}
}

