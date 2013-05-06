#ifndef SIMPLEIMAGE_H
#define SIMPLEIMAGE_H

#include "abstractimage.h"

#include <QPixmap>

class SimpleImage : public AbstractImage
{
public:
	SimpleImage(const QPixmap *pixmap, QString desc);

	QString getFileName() const;
	bool upload(AbstractUploader *uploader);
	QString toForumCode() const;

	bool isSelectedForSending() const;

private:
	const QPixmap *pixmap;
	QString desc;
};

#endif // SIMPLEIMAGE_H
