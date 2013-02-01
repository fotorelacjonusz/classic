#ifndef FTPUPLOADER_H
#define FTPUPLOADER_H

#include "abstractuploader.h"
#include <QFtp>
#include <QEventLoop>

namespace Ui {
class FtpUploader;
}

class FtpUploader : public AbstractUploader
{
	Q_OBJECT
	
public:
	explicit FtpUploader(QWidget *parent, QSettings &settings);
	virtual ~FtpUploader();
	virtual bool init(int imageNumber);
	virtual QString uploadImage(QString filePath, QIODevice *image);
	virtual void abort();
	virtual void finalize();

protected:
	bool advance();

private:
	Ui::FtpUploader *ui;
	QFtp ftp;
	QEventLoop loop;
	QString dirName;
};

#endif // FTPUPLOADER_H
