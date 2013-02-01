#include "simpleimage.h"
#include "abstractuploader.h"
#include "imagewidget.h"
#include <QBuffer>

SimpleImage::SimpleImage(const QPixmap *pixmap, QString desc):
	pixmap(pixmap),
	desc(desc)
{
}

QString SimpleImage::getFileName() const
{
	return desc;
}

bool SimpleImage::upload(AbstractUploader *uploader)
{
	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);
	pixmap->save(&buffer, "JPG");
	buffer.close();

	url = uploader->uploadImage(desc, &buffer);
	return !url.isEmpty();
}

QString SimpleImage::toForumCode() const
{
	QString text = url.isEmpty() ? ImageWidget::tr("Upload zdjęcia %1 nieudany.\n").arg(desc) : QString("[img]%1[/img]\n").arg(url);
	return appendPrepend(text) + "\n";
}

