#pragma once

#include "abstractuploader.h"
#include <QtFtp/QFtp>
#include <QEventLoop>

namespace Ui {
class FtpUploader;
}

/**
 * @brief Handles uploads to FTP servers.
 *
 * This class is responsible for uploader configuration as well as very uploads.
 *
 * It relies on QtFtp module, which has been removed from Qt, but is still
 * available for legacy software.
 *
 * @todo Migrate to supported Qt API.
 */
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
