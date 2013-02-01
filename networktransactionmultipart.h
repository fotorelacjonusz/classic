#ifndef NETWORKTRANSACTIONMULTIPART_H
#define NETWORKTRANSACTIONMULTIPART_H

#include "networktransaction.h"

#include <QHttpMultiPart>

class NetworkTransactionMultiPart : public NetworkTransaction
{
public:
	NetworkTransactionMultiPart(AbstractUploader *parent, QString url);

	void setBoundary(QByteArray boundary);
	void addHttpPart(QString name, QByteArray data);
	void addHttpPart(QString name, QString fileName, QIODevice *data);

	bool post();
	bool put();

protected:
	QHttpMultiPart multiPart;
};

#endif // NETWORKTRANSACTIONMULTIPART_H
