#ifndef ISCODEUPLOADER_H
#define ISCODEUPLOADER_H

#include "isanonyuploader.h"

namespace Ui {
class IsCodeUploader;
}

class IsCodeUploader : public IsAnonyUploader
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
