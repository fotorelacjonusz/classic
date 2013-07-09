#include "abstractimage.h"
#include "settings/settingsdialog.h"
#include <QDebug>

AbstractImage::~AbstractImage()
{
}

QString AbstractImage::fileName() const
{
	return QObject::tr("mapa.jpg");
}

QString AbstractImage::caption() const
{
	return "";
}

int AbstractImage::number() const
{
	return 0;
}

void AbstractImage::serialize(QDataStream &stream) const
{
	Q_UNUSED(stream);
}

QString AbstractImage::url() const
{
	return m_url;
}

void AbstractImage::setUrl(QString url)
{
	m_url = url;
}

QString AbstractImage::toBBCode() const
{
	AddFunc add = (SETTINGS->captionsUnder ? (AddFunc)&QString::append : (AddFunc)&QString::prepend);
	
	QString code = QString("[img]%1[/img]").arg(m_url);
		
	(code.*add)("\n");
	if (SETTINGS->extraSpace)
		(code.*add)("\n");
	(code.*add)(caption().trimmed());
	
	code.prepend(header + "\n");
	code.append("\n\n" + footer);
	return code.trimmed();
}

void AbstractImage::setHeader(QString text)
{
	header = text.trimmed();
}

void AbstractImage::setFooter(QString text)
{
	footer = text.trimmed();
}

