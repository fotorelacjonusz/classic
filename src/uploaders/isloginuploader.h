#ifndef ISLOGINUPLOADER_H
#define ISLOGINUPLOADER_H

/* TODO Fix and re-enable me!
 *
 * All ImageShack uploaders are all totally broken, and have been disabled:
 * - They do not work.
 * - They do not use current API.
 * - Amount of required changes is unknown.
 */
#if 0

#include "isanonuploader.h"

namespace Ui {
class IsLoginUploader;
}

class IsLoginUploader : public IsAnonUploader
{
	Q_OBJECT

public:
	explicit IsLoginUploader(QWidget *parent, QSettings &settings);
	virtual ~IsLoginUploader();
	virtual QString uploadImage(QString filePath, QIODevice *image);

private:
	Ui::IsLoginUploader *ui;
};

#endif // 0
#endif // ISLOGINUPLOADER_H
