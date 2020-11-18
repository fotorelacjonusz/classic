#pragma once

#include <QString>
#include <QMap>

class NetworkTransaction;

class ImgurResponse
{
	void parseResponse(const NetworkTransaction &tr);

public:
	ImgurResponse(const NetworkTransaction &tr);
	void debug() const;

	// TODO: Make it <QString, QVariant> to avoid unnecessary data conversions.
	QMap<QString, QString> data;
	bool success;
	int status;
	QString error;
	QString mergedError;
};
