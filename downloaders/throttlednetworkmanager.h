#ifndef THROTTLEDNETWORKMANAGER_H
#define THROTTLEDNETWORKMANAGER_H

#include <QQueue>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>

class AbstractMapDownloader;
class QNetworkReply;

class ThrottledNetworkManager : public QObject
{
	Q_OBJECT
	
public:
	explicit ThrottledNetworkManager(int limit, QObject *parent = 0);
	void get(QNetworkRequest request, AbstractMapDownloader *downloader);
	
public slots:
	void finished(QNetworkReply *reply);
	
protected:
	void start();
		
private:
	QNetworkAccessManager manager;
	QNetworkDiskCache cache;
	const int limit;
	int running;
	QQueue<QNetworkRequest> requests;
};

#endif // THROTTLEDNETWORKMANAGER_H
