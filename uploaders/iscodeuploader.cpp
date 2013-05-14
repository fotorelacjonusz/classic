#include "iscodeuploader.h"
#include "ui_iscodeuploader.h"
#include "networktransactionmultipart.h"
#include <QSettings>

IsCodeUploader::IsCodeUploader(QWidget *parent, QSettings &settings) :
	IsAnonyUploader(parent, settings),
	ui(new Ui::IsCodeUploader)
{
	ui->setupUi(this);
//	fixLayout(ui->formLayout);
	manager.makeInput("imageshack/code", ui->codeEdit);
}

IsCodeUploader::~IsCodeUploader()
{
	delete ui;
}

QString IsCodeUploader::uploadImage(QString filePath, QIODevice *image)
{
	NetworkTransactionMultiPart *tr = createTransaction(filePath, image);
	tr->addHttpPart("cookie", ui->codeEdit->text().toAscii());
	return postTransaction(tr, image);
}

