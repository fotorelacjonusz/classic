#include "networktransactionquery.h"

NetworkTransactionQuery::NetworkTransactionQuery(AbstractUploader *parent, QString url):
	NetworkTransaction(parent, url)
{
	setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
}

void NetworkTransactionQuery::addQueryItem(const QString &key, const QString &value)
{
	queryData.addQueryItem(key, value);
}

bool NetworkTransactionQuery::post()
{
	return finish(manager.post(request, queryData.encodedQuery()));
}

bool NetworkTransactionQuery::put()
{
	return finish(manager.put(request, queryData.encodedQuery()));
}
