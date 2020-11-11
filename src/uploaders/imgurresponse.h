#ifndef JSONOBJECT_H
#define JSONOBJECT_H

#include <QString>
#include <QMap>

class NetworkTransaction;

typedef QMap<QString, QString> ParamMap;

class ImgurResponse : public ParamMap
{
public:
	ImgurResponse(const NetworkTransaction &tr);
	void debug() const;

	ParamMap data;
	bool success;
	int status;
	QString error;
	QString mergedError;
};

#endif // JSONOBJECT_H
