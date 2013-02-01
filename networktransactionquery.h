#ifndef NETWORKTRANSACTIONQUERY_H
#define NETWORKTRANSACTIONQUERY_H

#include "networktransaction.h"

#include <QUrl>

class NetworkTransactionQuery : public NetworkTransaction
{
public:
	NetworkTransactionQuery(AbstractUploader *parent, QString url);

	void addQueryItem(const QString &key, const QString &value);

	bool post();
	bool put();

protected:
	QUrl queryData;
};

#endif // NETWORKTRANSACTIONQUERY_H
