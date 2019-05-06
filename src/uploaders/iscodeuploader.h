#ifndef ISCODEUPLOADER_H
#define ISCODEUPLOADER_H

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
class IsCodeUploader;
}

class IsCodeUploader : public IsAnonUploader
{
	Q_OBJECT

public:
	explicit IsCodeUploader(QWidget *parent, QSettings &settings);
	virtual ~IsCodeUploader();
	virtual QString uploadImage(QString filePath, QIODevice *image);

private:
	Ui::IsCodeUploader *ui;
};

#endif // 0
#endif // ISCODEUPLOADER_H
