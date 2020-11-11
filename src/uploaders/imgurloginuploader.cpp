#include "imgurloginuploader.h"
#include "ui_imguranonuploader.h"
#include "ui_imgurloginuploader.h"
#include "networkcookiejar.h"
#include "jsonobject.h"
#include "networktransactionquery.h"
#include "secrets.h"

#include <QWebElement>
#include <QWebFrame>
#include <QDebug>
#include <QNetworkReply>

#define CLIENT_ID_CSTR     "client_id"
#define CLIENT_SECRET_CSTR "client_secret"
#define GRANT_TYPE_CSTR    "grant_type"
#define	REFRESH_TOKEN_CSTR "refresh_token"
#define	ACCESS_TOKEN_CSTR  "access_token"
#define	EXPIRES_IN_CSTR    "expires_in"
#define	REDIRECT_URI_CSTR  "redirect_uri"
#define RESPONSE_TYPE_CSTR "response_type"
#define TOKEN_CSTR		   "token"

const QString ImgurLoginUploader::resPage = "qrc:/res/page.html";

ImgurLoginUploader::ImgurLoginUploader(QWidget *parent, QSettings &settings) :
	ImgurAnonUploader(parent, settings),
	ui(new Ui::ImgurLoginUploader)
{
	ui->setupUi(this);
	fixLayout(ui->formLayout);

	ui->webView->page()->networkAccessManager()->setCookieJar(new NetworkCookieJar("imgur/cookies"));
	connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(pageFinished()));

	manager.makeInput("imgur/refresh_token", &refreshToken);
}

ImgurLoginUploader::~ImgurLoginUploader()
{
	delete ui;
}

void ImgurLoginUploader::load()
{
	AbstractUploader::load();
	if (checkToken())
		updateCredits();
}

bool ImgurLoginUploader::init(int imageNumber)
{
	if (!checkToken())
		return false;
	if (!checkCredits(imageNumber, 1))
		return false;

	NetworkTransactionQuery transaction(this, "https://api.imgur.com/3/album/");
	setAuthorization(&transaction);
	transaction.addQueryItem("title", generateFolderName());
	transaction.post();
	JsonObject json(transaction);
	error = json.mergedError;
	if (transaction.success)
		albumId = json.data["id"];
	return transaction.success;
}

QString ImgurLoginUploader::uploadImage(QString filePath, QIODevice *image)
{
	if (!checkToken())
		return "";

	return ImgurAnonUploader::uploadImage(filePath, image);
}

bool ImgurLoginUploader::checkToken()
{
	if (expires.isValid() && QDateTime::currentDateTime().secsTo(expires) >= 60 && !accessToken.isEmpty())
	{
		ui->webView->load(QUrl(resPage + "?state=2"));
		return true;
	}

	if (refreshToken.isEmpty())
	{
		authorize();
		return false;
	}

	NetworkTransactionQuery transaction(this, "https://api.imgur.com/oauth2/token");
	transaction.addQueryItem(REFRESH_TOKEN_CSTR, refreshToken);
	transaction.addQueryItem(CLIENT_ID_CSTR, IMGUR_CLIENT_ID);
	transaction.addQueryItem(CLIENT_SECRET_CSTR, IMGUR_CLIENT_SECRET);
	transaction.addQueryItem(GRANT_TYPE_CSTR, REFRESH_TOKEN_CSTR);
	transaction.post();

	error = transaction.error;
	JsonObject json(transaction);
//	json.debug();

	if (json.contains(ACCESS_TOKEN_CSTR) && json.contains(REFRESH_TOKEN_CSTR) && json.contains(EXPIRES_IN_CSTR))
	{
		accessToken = json[ACCESS_TOKEN_CSTR];
		refreshToken = json[REFRESH_TOKEN_CSTR];
		expires = QDateTime::currentDateTime().addSecs(json[EXPIRES_IN_CSTR].toInt());
//		qDebug() << accessToken << refreshToken << expires;
		ui->webView->load(QUrl(resPage + "?state=2"));
		return true;
	}
	else
	{
//		qDebug() << error << json.error;
		ui->webView->load(QUrl(resPage + "?state=3&error=" + error + "\n<span class=\"accent green\">" + json.error + "</span>"));
		if (json.error == "Invalid refresh token")
		{
			refreshToken = "";
			authorize();
		}
		error = json.mergedError;
		return false;
	}
}

void ImgurLoginUploader::authorize()
{
	QUrl url("https://api.imgur.com/oauth2/authorize");
	url.addQueryItem(CLIENT_ID_CSTR, IMGUR_CLIENT_ID);
	url.addQueryItem(REDIRECT_URI_CSTR, resPage);
	url.addQueryItem(RESPONSE_TYPE_CSTR, TOKEN_CSTR);
	url.addQueryItem("state", "0");

	ui->webView->load(url);
}

void ImgurLoginUploader::pageFinished()
{
	QWebFrame *frame = ui->webView->page()->mainFrame();
//	qDebug() << ui->webView->url();
	QUrl url = ui->webView->url();
	if (url.toString().startsWith(resPage))
	{
//		qDebug() << "token refreshed:" << url";

		QMap<QString, QString> values;
		typedef QPair<QString, QString> QStringPair;
		foreach (QStringPair pair, QUrl(url.toString().replace("#", "&")).queryItems())
			values[pair.first] = pair.second;
		int state = values["state"].toInt();
		if (state == 0 && values.contains("error"))
			state = 1;

		if (state == 0)
		{
			expires = QDateTime::currentDateTime().addSecs(values[EXPIRES_IN_CSTR].toInt());
			accessToken = values[ACCESS_TOKEN_CSTR];
			refreshToken = values[REFRESH_TOKEN_CSTR];
			if (credits.userRemaining < 0)
				updateCredits();
		}

		static const QList<QPair<QString, QString> > titles = QList<QPair<QString, QString> >()
				<< qMakePair(tr("Autoryzacja udana"),  tr("Aplikacja <span class=\"accent green\">fotorelacjonusz</span> uzyskała dostęp do Twoich danych na Imgur."))
				<< qMakePair(tr("Odmowa autoryzacji"), tr("Aplikacja <span class=\"accent green\">fotorelacjonusz</span> nie uzyskała dostępu do Twoich danych na Imgur."))
				<< qMakePair(tr("Ok"),                 tr("Aplikacja <span class=\"accent green\">fotorelacjonusz</span> ma dostęp do Twoich danych na Imgur."))
				<< qMakePair(tr("Błąd"),               tr("Podczas kontaktowania się z Imgur wystąpił błąd."));

		frame->findFirstElement("#title").setInnerXml(titles[state].first);
		frame->findFirstElement("#title_long").setInnerXml(titles[state].second);
		frame->findFirstElement("#text").setInnerXml(values["error"]);
		if (state != 1)
			frame->findFirstElement("#buttons").setInnerXml("");
	}
	else
	{
		QWebElement topbar = frame->findFirstElement("#topbar");
		QWebElement header = frame->findFirstElement("#topbar > .header-center");
		if (!topbar.isNull() && !header.isNull())
		{
			topbar.setStyleProperty("min-width", "600px");
			header.setStyleProperty("width", "600px");
		}
	}
}

void ImgurLoginUploader::setAuthorization(NetworkTransaction *tr)
{
	tr->setRawHeader("Authorization", QString("Bearer " + accessToken).toAscii());
}
