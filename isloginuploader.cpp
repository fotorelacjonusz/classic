#include "isloginuploader.h"
#include "ui_isloginuploader.h"
#include <QSettings>

IsLoginUploader::IsLoginUploader(QWidget *parent, QSettings &settings) :
	IsAnonyUploader(parent, settings),
	ui(new Ui::IsLoginUploader)
{
	ui->setupUi(this);
//	fixLayout(ui->formLayout);
	manager.makeInput("imageshack/login", ui->loginEdit);
	manager.makeInput("imageshack/password", ui->passwordEdit);
}

IsLoginUploader::~IsLoginUploader()
{
	delete ui;
}

QString IsLoginUploader::uploadImage(QString filePath, QIODevice *image)
{
	QString login = ui->loginEdit->text();
	QString password = ui->passwordEdit->text();
	if (password.isEmpty())
		password = queryPassword(tr("%1 na imageshack").arg(login));
	if (login.isEmpty() || password.isEmpty())
		return QString();

	NetworkTransactionMultiPart *tr = createTransaction(filePath, image);
	tr->addHttpPart("a_username", login.toAscii());
	tr->addHttpPart("a_password", password.toAscii());
	return postTransaction(tr, image);
}

