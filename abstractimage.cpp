#include "abstractimage.h"

AbstractImage::~AbstractImage()
{
}

void AbstractImage::serialize(QDataStream &stream) const
{
	Q_UNUSED(stream);
}

QString AbstractImage::getUrl() const
{
	return url;
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
