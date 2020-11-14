#pragma once

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
	explicit ThrottledNetworkManager(int limit = 100, QObject *parent = nullptr);
	void get(QNetworkRequest request, AbstractMapDownloader *downloader);

	QNetworkAccessManager *manager();
public slots:
	void finished(QNetworkReply *reply);

protected:
	void start();

private:
	QNetworkAccessManager m_manager;
	QNetworkDiskCache cache;
	const int limit;
	int running;
	QQueue<QNetworkRequest> requests;
};
