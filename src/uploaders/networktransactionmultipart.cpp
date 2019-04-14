#include "networktransactionmultipart.h"

#include <QRegExp>

NetworkTransactionMultiPart::NetworkTransactionMultiPart(AbstractUploader *parent, QString url):
	NetworkTransaction(parent, url),
	multiPart(QHttpMultiPart::FormDataType)
{
}

void NetworkTransactionMultiPart::setBoundary(QByteArray boundary)
{
	multiPart.setBoundary(boundary);
}

void NetworkTransactionMultiPart::addHttpPart(QString name, QByteArray data)
{
	QHttpPart part;
	part.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"%1\"").arg(name));
	part.setBody(data);
	multiPart.append(part);
}

void NetworkTransactionMultiPart::addHttpPart(QString name, QString fileName, QIODevice *data)
{
	fileName.replace(QRegExp("\\..*$"), ".jpg");
	QHttpPart part;
	part.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"%1\"; filename=\"%2\"").arg(name).arg(fileName));
	part.setHeader(QNetworkRequest::ContentTypeHeader, "image/jpeg");
	part.setBodyDevice(data);
	multiPart.append(part);
}

bool NetworkTransactionMultiPart::post()
{
	return finish(manager.post(request, &multiPart));
}

bool NetworkTransactionMultiPart::put()
{
	return finish(manager.put(request, &multiPart));
}
