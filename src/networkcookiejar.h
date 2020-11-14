#pragma once

#include <QNetworkCookieJar>
#include <QNetworkCookie>

class QSettings;

class NetworkCookieJar : public QNetworkCookieJar
{
	Q_OBJECT
	
public:
	explicit NetworkCookieJar(QString key = "cookies");
	~NetworkCookieJar();
	
private:
	QString key;
	QSettings &settings;
};
