#include "imguranonyuploader.h"
#include "ui_imguranonyuploader.h"
#include "jsonobject.h"
#include "networktransactionmultipart.h"
#include "secrets.h"

#include <QDebug>

#define USER_LIMIT_CSTR     "UserLimit"        // Total credits that can be allocated.
#define USER_REMAINING_CSTR "UserRemaining"    // Total credits available.
#define USER_RESET_CSTR     "UserReset"        // Timestamp (unix epoch) for when the credits will be reset.1
#define APP_LIMIT_CSTR      "ClientLimit"      // Total credits that can be allocated for the application in a day.
#define APP_REMAINING_CSTR  "ClientRemaining"  // Total credits remaining for the application in a day.


ImgurAnonyUploader::Credits::Credits():
	userLimit(-1), userRemaining(-1), appLimit(-1), appRemaining(-1)
{
}

ImgurAnonyUploader::ImgurAnonyUploader(QWidget *parent, QSettings &settings) :
	AbstractUploader(parent, settings),
	ui(new Ui::ImgurAnonyUploader)
{
	ui->setupUi(this);
}

ImgurAnonyUploader::~ImgurAnonyUploader()
{
	delete ui;
}

void ImgurAnonyUploader::load()
{
	AbstractUploader::load();
	updateCredits();
}

bool ImgurAnonyUploader::init(int imageNumber)
{
	return checkCredits(imageNumber);
}

QString ImgurAnonyUploader::uploadImage(QString filePath, QIODevice *image)
{
	image->open(QIODevice::ReadOnly);
	NetworkTransactionMultiPart tr(this, "https://api.imgur.com/3/image");
	setAuthorization(&tr);
	connect(&tr, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(uploadProgress(qint64,qint64)));
	tr.addHttpPart("image", filePath, image);
	tr.addHttpPart("type", "file");
	if (!albumId.isEmpty())
		tr.addHttpPart("album_id", albumId.toAscii());
	tr.post();

	JsonObject json(tr);
	error = json.mergedError;
	image->close();
	return json.success ? json.data["link"] : "";
}

QString ImgurAnonyUploader::tosUrl() const
{
	return "http://imgur.com/tos";
}

void ImgurAnonyUploader::updateCredits()
{
	NetworkTransaction tr(this, "https://api.imgur.com/3/credits");
	setAuthorization(&tr);
	tr.get();
	qDebug() << "credits updated";
	JsonObject json(tr);

	credits.userLimit = json.data[USER_LIMIT_CSTR].toInt();
	credits.userRemaining = json.data[USER_REMAINING_CSTR].toInt();
	credits.appLimit = json.data[APP_LIMIT_CSTR].toInt();
	credits.appRemaining = json.data[APP_REMAINING_CSTR].toInt();

	ui->userReset->setDateTime(QDateTime::fromTime_t(json.data[USER_RESET_CSTR].toInt()));
	ui->userCredits->setText(QString("%1 / %2").arg(credits.userRemaining).arg(credits.userLimit));
	ui->appCredits->setText(QString("%1 / %2").arg(credits.appRemaining).arg(credits.appLimit));

	if (credits.userLimit == 0 || credits.userRemaining == 0 ||
		credits.appLimit == 0 || credits.appRemaining == 0)
	{
		json.debug();
		qDebug() << ui->userCredits->text() << ui->appCredits->text();
	}
}

bool ImgurAnonyUploader::checkCredits(int imageNumber, int extra)
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

void ImgurAnonyUploader::setAuthorization(NetworkTransaction *tr)
{
	tr->setRawHeader("Authorization", QString("Client-ID " IMGUR_CLIENT_ID).toAscii());
}
