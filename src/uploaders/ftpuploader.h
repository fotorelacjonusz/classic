#ifndef FTPUPLOADER_H
#define FTPUPLOADER_H

/* TODO Re-enable me!
 *
 * FTP uploader has been disabled due to removal of QFtp class from Qt.
 *
 * Possible solutions:
 * - QFtp can be obtained from https://code.qt.io/cgit/qt/qtftp.git/.
 *   It has been extracted specifically for legacy projects.  However,
 *   this repository is hardly maintained, and compiling that source
 *   code is surprisingy difficult.
 * - QFtp can be replaced with QNetworkAccessManager, which provides
 *   some basic FTP interface.  This class is part of Qt, and is supported.
 *   Hopefully, that limited feature set will be enough, as we don't
 *   need anything fancy.
 * - Another 3rd-party replacement.
 */
#if 0

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

*/

#endif // 0
#endif // FTPUPLOADER_H
