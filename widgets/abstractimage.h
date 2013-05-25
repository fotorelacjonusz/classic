#ifndef ABSTRACTIMAGE_H
#define ABSTRACTIMAGE_H

#include <QString>

class QIODevice;

class AbstractImage
{
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
	
	void setHeader(QString text);
	void setFooter(QString text);

protected:
	QString m_url;

private:
	QString header;
	QString footer;
};

#endif // ABSTRACTIMAGE_H
