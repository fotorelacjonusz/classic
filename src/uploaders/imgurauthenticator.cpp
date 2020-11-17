#include "imgurauthenticator.h"
#include "secrets.h"

#include <QDesktopServices>
#include <QWebSocket>
#include <QWebSocketServer>

const QString serverName("Fotorelacjonusz Imgur Authenticator listener");

const QUrl authorizeUrl("https://api.imgur.com/oauth2/authorize"
						"?response_type=token"
						"&state=classic-v1"
						"&client_id=" IMGUR_CLIENT_ID);

const quint16 callbackPort = 1235;

ImgurAuthenticator::ImgurAuthenticator():
	callbackListener(serverName, QWebSocketServer::NonSecureMode, this)
{
	connect(&callbackListener, SIGNAL(newConnection()),
			this, SLOT(handleNewConnection()));
}

/**
 * @brief Initializes authorization flow.
 * It launches browser and starts listening for access token.
 */
void ImgurAuthenticator::grantAccess()
{
	listenForToken();
	openAuthPage();
}

void ImgurAuthenticator::openAuthPage()
{
	QDesktopServices::openUrl(authorizeUrl);
}

void ImgurAuthenticator::listenForToken()
{
	//TODO launch timer
	//TODO it can pick port automatically
	callbackListener.listen(QHostAddress::LocalHost, callbackPort);
}

void ImgurAuthenticator::stopListeningForToken()
{
	//TODO stop timer if running
	callbackListener.close();
}

void ImgurAuthenticator::handleNewConnection()
{
	// TODO Close socket, perhaps?
	// Not many of them though, let them leak, perhaps?
	// Or are they auto-closed on server shutdown?
	QWebSocket *socket = callbackListener.nextPendingConnection();
	connect(socket, SIGNAL(textMessageReceived(QString)),
			this, SLOT(handleTokenMessage(QString)));
}

void ImgurAuthenticator::handleTokenMessage(QString message)
{
	stopListeningForToken();
	Credentials cr = parseAccessTokenMessage(message);
	emit(accessGranted(cr));
}


/**
 * @brief Parses token message which has been received from Imgur on callback
 * page.
 * @param message "access_token=xyz&..." as received from Imgur
 * @return Parsed credentials
 */
ImgurAuthenticator::Credentials ImgurAuthenticator::parseAccessTokenMessage(const QString message)
{
	QUrlQuery payload(message);
	Credentials retval;

	Q_ASSERT(payload.queryItemValue("token_type") == "bearer");

	auto expiresIn = payload.queryItemValue("expires_in").toUInt();
	retval.accessTokenExpiresAt =
			QDateTime::currentDateTime().addSecs(expiresIn);
	retval.accessToken = payload.queryItemValue("access_token");
	retval.refreshToken = payload.queryItemValue("refresh_token");
	retval.userName = payload.queryItemValue("account_username");

	return retval;
}
