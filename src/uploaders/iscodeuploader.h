#ifndef ISCODEUPLOADER_H
#define ISCODEUPLOADER_H

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

#endif // ISCODEUPLOADER_H
