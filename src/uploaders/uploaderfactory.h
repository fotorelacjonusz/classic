#pragma once

#include <QString>
#include <QList>
#include <QPointer>

class AbstractUploader;
class QWidget;
class QSettings;

class UploaderFactory : public QObject
{
	Q_OBJECT

public:
	class AbstractWrapper
	{
	public:
		AbstractWrapper(QString name);
		virtual ~AbstractWrapper();
		virtual AbstractUploader *make(QWidget *w, QSettings &s) = 0;
		const QString name;
	};

public:
	UploaderFactory();
	virtual ~UploaderFactory();

	QList <AbstractWrapper *> uploaders;

};
