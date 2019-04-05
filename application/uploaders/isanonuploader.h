#ifndef ISANONYUPLOADER_H
#define ISANONYUPLOADER_H

/* TODO Fix and re-enable me!
 *
 * All ImageShack uploaders are all totally broken, and have been disabled:
 * - They do not work.
 * - They do not use current API.
 * - Amount of required changes is unknown.
 */
#if 0

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

#endif // 0
#endif // ISANONYUPLOADER_H
