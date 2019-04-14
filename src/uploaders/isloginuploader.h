#ifndef ISLOGINUPLOADER_H
#define ISLOGINUPLOADER_H

#include "isanonyuploader.h"

namespace Ui {
class IsLoginUploader;
}

class IsLoginUploader : public IsAnonyUploader
{
	Q_OBJECT

public:
	explicit IsLoginUploader(QWidget *parent, QSettings &settings);
	virtual ~IsLoginUploader();
	virtual QString uploadImage(QString filePath, QIODevice *image);

private:
	Ui::IsLoginUploader *ui;
};

#endif // ISLOGINUPLOADER_H
