#include "abstractimage.h"
#include <QDebug>

AbstractImage::~AbstractImage()
{
}

void AbstractImage::serialize(QDataStream &stream)
{
	Q_UNUSED(stream);
}

QString AbstractImage::getUrl() const
{
	return url;
}

int AbstractImage::getNumber() const
{
	return 0;
}

void AbstractImage::prepend(QString text)
{
	prependedText = text.trimmed();
}

void AbstractImage::append(QString text)
{
	appendedText = text.trimmed();
}

QString AbstractImage::appendPrepend(QString text) const
{
	if (!prependedText.isEmpty())
		text.prepend(prependedText + "\n\n");
	if (!appendedText.isEmpty())
		text.append("\n" + appendedText + "\n");
	return text;
}
