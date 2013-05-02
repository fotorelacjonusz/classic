#ifndef THROTTLEDNETWORKMANAGER_H
#define THROTTLEDNETWORKMANAGER_H

#include <QQueue>
#include <QPair>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>

class QNetworkAccessManager;
class AbstractMapDownloader;
class QNetworkReply;

class ThrottledNetworkManager : public QObject
{
	Q_OBJECT
	
public:
	explicit ThrottledNetworkManager(int limit, QObject *parent = 0);
	void get(QNetworkRequest request, AbstractMapDownloader *downloader);
	
signals:
	
public slots:
	void finished(QNetworkReply *reply);
	
protected:
	void start();
		
private:
	QNetworkAccessManager manager;
	QNetworkDiskCache cache;
	const int limit;
	int running;
	QHash<QNetworkReply *, AbstractMapDownloader *> listeners;
	QQueue<QPair<QNetworkRequest, AbstractMapDownloader *> > requests;
};

#endif // THROTTLEDNETWORKMANAGER_H
