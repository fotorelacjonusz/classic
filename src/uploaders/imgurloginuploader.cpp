#include "imgurloginuploader.h"
#include "ui_imguranonuploader.h"
#include "ui_imgurloginuploader.h"
#include "networkcookiejar.h"
#include "networktransactionquery.h"
#include "imgurresponse.h"
#include "secrets.h"

#include <QDebug>

ImgurLoginUploader::ImgurLoginUploader(QWidget *parent, QSettings &settings) :
	ImgurAnonUploader(parent, settings),
	ui(new Ui::ImgurLoginUploader)
{
	ui->setupUi(this);
	fixLayout(ui->formLayout);

	connect(ui->authorize, SIGNAL(clicked()), this,
			SLOT(requestAuthorization()));

	connect(&auth, SIGNAL(accessGranted(Credentials)),
			this, SLOT(finalizeAuthorization(Credentials)));
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
	qDebug() << "init uploader " << imageNumber;
	if (!checkToken()) {
		error = tr("Wejdź w ustawienia programu, zaloguj się do Imgura "
				   "i spróbuj raz jeszcze");
		return false;
	}
	qDebug() << "checking credits";
	if (!checkCredits(imageNumber, 1))
		return false;

	qDebug() << "proceeding";

	// TODO Extract album creation.
	NetworkTransactionQuery transaction(this, "https://api.imgur.com/3/album/");
	setAuthorization(&transaction);
	transaction.addQueryItem("title", generateFolderName());
	transaction.post();
	ImgurResponse json(transaction);
	error = json.mergedError;
	if (transaction.success)
		albumId = json.data["id"].toString();
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
	return credentials.areValid();
	// TODO autorhize or refresh
}

void ImgurLoginUploader::requestAuthorization()
{
	auth.grantAccess();
}

void ImgurLoginUploader::finalizeAuthorization(Credentials cr)
{
	qDebug() << "Authorized as " << cr.userName << ".";
	credentials = cr;
	updateLoginInfo();
}

/**
 * @brief Updates uploader's UI with proper login information.
 */
void ImgurLoginUploader::updateLoginInfo()
{
	ui->userName->setText(credentials.userName);

	if (credits.userRemaining < 0) // TODO WTF?
		updateCredits();
}

void ImgurLoginUploader::setAuthorization(NetworkTransaction *tr)
{
	tr->setRawHeader("Authorization", QString("Bearer " + credentials.accessToken).toLatin1());
}
