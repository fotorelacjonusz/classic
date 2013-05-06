#ifndef ABSTRACTIMAGE_H
#define ABSTRACTIMAGE_H

#include <QString>

class AbstractUploader;

class AbstractImage
{
public:
	virtual ~AbstractImage();

	virtual QString getFileName() const = 0;
	virtual bool upload(AbstractUploader *uploader) = 0;
	virtual QString toForumCode() const = 0;

	QString getUrl() const;
	virtual int getNumber() const;
	virtual void serialize(QDataStream &stream) const;

	void prepend(QString text);
	void append(QString text);

protected:
	QString appendPrepend(QString text) const;

	QString url;

private:
	QString prependedText;
	QString appendedText;
};

#endif // ABSTRACTIMAGE_H
