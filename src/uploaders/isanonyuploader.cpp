#include "isanonyuploader.h"
#include "ui_isanonyuploader.h"
#include "networktransaction.h"
#include "networktransactionmultipart.h"
#include "secrets.h"

#include <QDebug>
#include <QEventLoop>
#include <QNetworkReply>

IsAnonyUploader::IsAnonyUploader(QWidget *parent, QSettings &settings) :
	AbstractUploader(parent, settings),
	ui(new Ui::IsAnonyUploader)
{
	ui->setupUi(this);
}

IsAnonyUploader::~IsAnonyUploader()
{
	delete ui;
}

QString IsAnonyUploader::uploadImage(QString filePath, QIODevice *image)
{
	NetworkTransactionMultiPart *tr = createTransaction(filePath, image);
	return postTransaction(tr, image);
}

QString IsAnonyUploader::tosUrl() const
{
	return "http://imageshack.us/p/rules/";
}

NetworkTransactionMultiPart *IsAnonyUploader::createTransaction(QString fileName, QIODevice *image)
{
	image->open(QIODevice::ReadOnly);
	NetworkTransactionMultiPart *tr = new NetworkTransactionMultiPart(this, "http://load5.imageshack.us/upload_api.php");
	tr->setRawHeader("User-Agent", "Fotorelacjonusz");
	connect(tr, SIGNAL(uploadProgress(qint64,qint64)), this, SIGNAL(uploadProgress(qint64,qint64)));
	tr->setBoundary("UPLOADERBOUNDARY");
	tr->addHttpPart("fileupload", fileName, image);
	tr->addHttpPart("public", "yes");
	tr->addHttpPart("key", IMAGESHACK_KEY);
	return tr;
}

QString IsAnonyUploader::postTransaction(NetworkTransactionMultiPart *tr, QIODevice *image)
{
	tr->post();
	error = tr->error;

	QRegExp errorExp("<error>(.+)</error>");
	if (tr->data.contains(errorExp))
		error = errorExp.cap(1);

	QString link = tr->data.section(QRegExp("</?image_link>"), 1, 1);
	if (link.isEmpty() && error.isEmpty())
		error = "No error.\nImageshack reply:\n" + tr->data;
	delete tr;
	image->close();
	return link;
}
