#include "networkcookiejar.h"
#include "settings/settingsdialog.h"

NetworkCookieJar::NetworkCookieJar(QString key) :
	QNetworkCookieJar(),
	key(key),
	settings(SETTINGS->settings())
{
	setAllCookies(QNetworkCookie::parseCookies(settings.value(key).toByteArray()));
}

NetworkCookieJar::~NetworkCookieJar()
{
	QByteArray cookies;
	foreach (QNetworkCookie cookie, allCookies())
		cookies.append(cookie.toRawForm() + ";\r\n");
	settings.setValue(key, cookies);
}
