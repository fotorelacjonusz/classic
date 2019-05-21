#ifndef ABSTRACTUPLOADER_H
#define ABSTRACTUPLOADER_H

#include "settings/settingsmanager.h"
#include <QWidget>
#include <QHttpPart>
#include <QMap>
#include <QList>
#include <QPair>
#include <QNetworkAccessManager>

class QSettings;
class QFormLayout;
class NetworkTransaction;

class AbstractUploader : public QWidget
{
	Q_OBJECT

public:
	AbstractUploader(QWidget *parent, QSettings &settings);
	virtual ~AbstractUploader();

public slots:
	virtual void load();

public:
	void save();

	virtual bool init(int imageNumber);
	virtual QString uploadImage(QString filePath, QIODevice *image) = 0; // blokujące
	virtual void abort();
	virtual void finalize();

	QWidget *lastInputWidget() const;
	QString lastError() const;
	virtual QString tosUrl() const;

signals:
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
	void abortRequested();

protected:
	static QString generateFolderName();
	static QString removeAccents(QString diacritical);

	QString queryPassword(QString sourceDsc, bool *ok = nullptr);

//	void fixLayout(QBoxLayout *child);
	void fixLayout(QFormLayout *child);
//	void fixLayout(QGridLayout *child);
//	void fixLayout(QStackedLayout *child);

	QSettings &settings;
	SettingsManager manager;

	QString error;
	QString tempPassword;
};


#endif // ABSTRACTUPLOADER_H
