#include "imguranonuploader.h"
#include "ui_imguranonuploader.h"
#include "imgurresponse.h"
#include "networktransactionmultipart.h"
#include "secrets.h"

#include <QDebug>

#define USER_LIMIT_CSTR     "UserLimit"        // Total credits that can be allocated.
#define USER_REMAINING_CSTR "UserRemaining"    // Total credits available.
#define USER_RESET_CSTR     "UserReset"        // Timestamp (unix epoch) for when the credits will be reset.1
#define APP_LIMIT_CSTR      "ClientLimit"      // Total credits that can be allocated for the application in a day.
#define APP_REMAINING_CSTR  "ClientRemaining"  // Total credits remaining for the application in a day.


ImgurAnonUploader::Credits::Credits():
	userLimit(-1), userRemaining(-1), appLimit(-1), appRemaining(-1)
{
}

ImgurAnonUploader::ImgurAnonUploader(QWidget *parent, QSettings &settings) :
	AbstractUploader(parent, settings),
	ui(new Ui::ImgurAnonUploader)
{
	ui->setupUi(this);
}

ImgurAnonUploader::~ImgurAnonUploader()
{
	delete ui;
}

void ImgurAnonUploader::load()
{
	AbstractUploader::load();
	updateCredits();
}

bool ImgurAnonUploader::init(int imageNumber)
{
	return checkCredits(imageNumber);
}

QString ImgurAnonUploader::uploadImage(QString filePath, QIODevice *image)
{
	image->open(QIODevice::ReadOnly);
	NetworkTransactionMultiPart tr(this, "https://api.imgur.com/3/image");
	setAuthorization(&tr);
	connect(&tr, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(uploadProgress(qint64,qint64)));
	tr.addHttpPart("image", filePath, image);
	tr.addHttpPart("type", "file");
	if (!albumId.isEmpty())
		tr.addHttpPart("album_id", albumId.toLatin1());
	tr.post();

	ImgurResponse json(tr);
	error = json.mergedError;
	image->close();
	return json.success ? json.data["link"].toString() : "";
}

QString ImgurAnonUploader::tosUrl() const
{
	return "http://imgur.com/tos";
}

void ImgurAnonUploader::updateCredits()
{
	NetworkTransaction tr(this, "https://api.imgur.com/3/credits");
	setAuthorization(&tr);
	tr.get();
	qDebug() << "credits updated";
	ImgurResponse json(tr);

	credits.userLimit = json.data[USER_LIMIT_CSTR].toInt();
	credits.userRemaining = json.data[USER_REMAINING_CSTR].toInt();
	credits.appLimit = json.data[APP_LIMIT_CSTR].toInt();
	credits.appRemaining = json.data[APP_REMAINING_CSTR].toInt();

	qint64 creditsResetInSecs = json.data[USER_RESET_CSTR].toLongLong();

	ui->userReset->setDateTime(QDateTime::fromSecsSinceEpoch(creditsResetInSecs));
	ui->userCredits->setText(QString("%1 / %2").arg(credits.userRemaining).arg(credits.userLimit));
	ui->appCredits->setText(QString("%1 / %2").arg(credits.appRemaining).arg(credits.appLimit));

	if (credits.userLimit == 0 || credits.userRemaining == 0 ||
		credits.appLimit == 0 || credits.appRemaining == 0)
	{
		json.debug();
		qDebug() << ui->userCredits->text() << ui->appCredits->text();
	}
}

bool ImgurAnonUploader::checkCredits(int imageNumber, int extra)
{
	updateCredits();
	int remaining = qMin(credits.userRemaining, credits.appRemaining);
	if (remaining < imageNumber * 10 + extra)
	{
		error = tr("Pozostała liczba kredytów: %1 nie wystarczy na upload %2 zdjęć. Zredukuj liczbę zdjęć do %3 i spróbuj jeszcze raz.")
				.arg(remaining).arg(imageNumber).arg((remaining - extra - 1) / 10);
		return false;
	}
	return true;
}

void ImgurAnonUploader::setAuthorization(NetworkTransaction *tr)
{
	tr->setRawHeader("Authorization", QString("Client-ID " IMGUR_CLIENT_ID).toLatin1());
}
