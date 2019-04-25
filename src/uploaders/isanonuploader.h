#ifndef ISANONYUPLOADER_H
#define ISANONYUPLOADER_H

#include "abstractuploader.h"

class NetworkTransactionMultiPart;

namespace Ui {
class IsAnonUploader;
}

class IsAnonUploader : public AbstractUploader
{
	Q_OBJECT

public:
	explicit IsAnonUploader(QWidget *parent, QSettings &settings);
	virtual ~IsAnonUploader();
	virtual QString uploadImage(QString filePath, QIODevice *image);
	virtual QString tosUrl() const;

protected:
	NetworkTransactionMultiPart *createTransaction(QString fileName, QIODevice *image);
	QString postTransaction(NetworkTransactionMultiPart *tr, QIODevice *image);

private:
	Ui::IsAnonUploader *ui;
};

#endif // ISANONYUPLOADER_H
