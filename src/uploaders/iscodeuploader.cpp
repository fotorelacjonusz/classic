/* TODO Re-enable me!  (See respective header file) */
#if 0

#include "iscodeuploader.h"
#include "ui_iscodeuploader.h"
#include "networktransactionmultipart.h"
#include <QSettings>

IsCodeUploader::IsCodeUploader(QWidget *parent, QSettings &settings) :
	IsAnonUploader(parent, settings),
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
	tr->addHttpPart("cookie", ui->codeEdit->text().toLatin1());
	return postTransaction(tr, image);
}

#endif // 0
