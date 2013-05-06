#ifndef ISANONYUPLOADER_H
#define ISANONYUPLOADER_H

#include "abstractuploader.h"
#include "networktransactionmultipart.h"

namespace Ui {
class IsAnonyUploader;
}

class IsAnonyUploader : public AbstractUploader
{
	Q_OBJECT
	
public:
	explicit IsAnonyUploader(QWidget *parent, QSettings &settings);
	virtual ~IsAnonyUploader();
	virtual QString uploadImage(QString filePath, QIODevice *image);
	virtual QString tosUrl() const;

protected:
	NetworkTransactionMultiPart *createTransaction(QString fileName, QIODevice *image);
	QString postTransaction(NetworkTransactionMultiPart *tr, QIODevice *image);

private:
	Ui::IsAnonyUploader *ui;
};

#endif // ISANONYUPLOADER_H
