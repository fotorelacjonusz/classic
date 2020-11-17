#pragma once

#include <QObject>
#include <QUrl>
#include <QWebSocketServer>

/**
 * @brief The ImgurAuthenticator class encapsulates Imgur authentication flow.
 *
 * Above all, it obtains an access token, which then can be used to perform
 * various actions on Imgur.com site on behalf of user who has authorized
 * that token.  It implements OAuth2 Implicit Grant, as described in RFC6749
 * sec. 4.2 (https://tools.ietf.org/html/rfc6749#section-4.2).
 *
 * This class manages a web server socket, which receives authentication
 * credentials from a callback page.  This is because authorization must be
 * performed in a browser, either embedded or external one (here the latter),
 * and web socket connection over localhost is a convenient way to communicate
 * with that browser.
 *
 * Authentication is performed according to the following algorithm:
 *
 * - Fotorelacjonusz launches a web socket server and waits for message.
 * - A system browser is navigated to authorization page on Imgur site.
 * - User signs in and grants access to Fotorelacjonusz application.
 * - Upon successful authorization, browser is redirected to a callback page,
 *   which is accessible from the Internet (deployed at
 *   fotorelacjonusz.github.io site).
 * - Callback page connects to Fotorelacjonusz over localhost via web socket,
 *   and sends credentials.
 * - accessGranted signal is emitted.
 *
 * @todo Proper message validation (e.g. authorization rejected).
 *
 * @todo Refreshing expired tokens (little important because token TTL is
 *   10 ys).
 *
 * @todo Kill web socket server after some time (5 minutes?).
 *
 * @todo Make sure there are no zombie sockets.
 *
 * @todo Use arbitrary port number instead of hardcoded one.
 */
class ImgurAuthenticator : public QObject
{
	Q_OBJECT

public:
	/// @brief Imgur credentials
	struct Credentials {
		QString userName;
		QString accessToken;
		QString refreshToken;
		QDateTime accessTokenExpiresAt;

		inline bool areValid() {
			return !accessToken.isEmpty();
		}
	};

private:
	QWebSocketServer callbackListener;

	void listenForToken();
	void stopListeningForToken();
	void openAuthPage();
	Credentials parseAccessTokenMessage(const QString);

public:
	ImgurAuthenticator();

	void grantAccess();

signals:
	void accessGranted(Credentials);

public slots:
	void handleNewConnection();
	void handleTokenMessage(QString);
};
