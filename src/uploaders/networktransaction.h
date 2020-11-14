#pragma once

#include <QString>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QEventLoop>

class QIODevice;
class QNetworkReply;
class QNetworkRequest;
class AbstractUploader;

class NetworkTransaction : public QObject
{
	Q_OBJECT

public:
	NetworkTransaction(AbstractUploader *parent, QString url);
	virtual ~NetworkTransaction();
	void setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value);
	void setRawHeader(const QByteArray &headerName, const QByteArray &value);

	bool head();
	bool get();

	bool deleteResource();
	bool sendCustomRequest(const QByteArray &verb, QIODevice *data = nullptr);

	QString error;
	QString data;
	QMap<QString, QString> rawHeaders;
	bool success;

public slots:
	void abort();

signals:
	void uploadProgress(qint64,qint64);

protected:
	bool finish(QNetworkReply *_reply);

	QNetworkAccessManager manager;
	QNetworkRequest request;
	QNetworkReply *reply;

	QEventLoop loop;
};
