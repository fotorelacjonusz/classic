#ifndef NETWORKCOOKIEJAR_H
#define NETWORKCOOKIEJAR_H

#include <QNetworkCookieJar>
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

#endif // NETWORKCOOKIEJAR_H
