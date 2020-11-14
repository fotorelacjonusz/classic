#pragma once

#include <QString>
#include <QCoreApplication>

class QIODevice;

class AbstractImage
{
	Q_DECLARE_TR_FUNCTIONS(AbstractImage)

public:
	enum State { Ready, Uploaded, Assigned };
	typedef QString &(QString::*AddFunc)(const QString &);

	virtual ~AbstractImage();

	virtual QString fileName() const;
	virtual QString caption() const;
	virtual int number() const;
	virtual void serialize(QDataStream &stream) const;
	virtual void write(QIODevice *device) const = 0;

	QString url() const;
	void setUrl(QString url);
	QString toBBCode() const;

private:
	QString m_url;
};
