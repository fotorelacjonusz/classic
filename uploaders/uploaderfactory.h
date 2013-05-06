#ifndef UPLOADERFACTORY_H
#define UPLOADERFACTORY_H

#include <QString>
#include <QPair>
#include <QMap>
#include <QPointer>

class AbstractUploader;
class QWidget;
class QSettings;

class UploaderFactory : public QObject
{
	Q_OBJECT

public:
	class AbstarctWrapper
	{
	public:
		AbstarctWrapper(QString name);
		virtual ~AbstarctWrapper();
		virtual AbstractUploader *make(QWidget *w, QSettings &s) = 0;
		const QString name;
	};

	template <class T>
	class Wrapper : public AbstarctWrapper
	{
	public:
		Wrapper(QString name);
		virtual ~Wrapper();
		AbstractUploader *make(QWidget *w, QSettings &s);

	private:
		QPointer<T> object;
	};


public:
	UploaderFactory();
	virtual ~UploaderFactory();

	QList <AbstarctWrapper *> uploaders;
private:

};

#endif // UPLOADERFACTORY_H
