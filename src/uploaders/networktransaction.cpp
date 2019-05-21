#include "networktransaction.h"
#include "abstractuploader.h"

#include <QHttpMultiPart>
#include <QNetworkReply>
#include <QDebug>

NetworkTransaction::NetworkTransaction(AbstractUploader *parent, QString url):
	QObject(parent),
	success(false),
	request(QUrl(url)),
	reply(nullptr)
{
	connect(parent, SIGNAL(abortRequested()), this, SLOT(abort()));
}

NetworkTransaction::~NetworkTransaction()
{
}

void NetworkTransaction::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
	request.setHeader(header, value);
}

void NetworkTransaction::setRawHeader(const QByteArray &headerName, const QByteArray &value)
{
	request.setRawHeader(headerName, value);
}

bool NetworkTransaction::head()
{
	return finish(manager.head(request));
}

bool NetworkTransaction::get()
{
	return finish(manager.get(request));
}

bool NetworkTransaction::deleteResource()
{
	return finish(manager.deleteResource(request));
}

bool NetworkTransaction::sendCustomRequest(const QByteArray &verb, QIODevice *data)
{
	return finish(manager.sendCustomRequest(request, verb, data));
}

void NetworkTransaction::abort()
{
	if (reply)
		reply->abort();
}

bool NetworkTransaction::finish(QNetworkReply *_reply)
{
	reply = _reply;
	connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(uploadProgress(qint64,qint64)));
	connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();

	success = reply->error() == QNetworkReply::NoError;
	if (!success)
		error = reply->errorString();
	foreach (QNetworkReply::RawHeaderPair pair, reply->rawHeaderPairs())
		rawHeaders[pair.first] = pair.second;

	data = reply->readAll();
	delete reply;
	reply = nullptr;
	return success;
}
