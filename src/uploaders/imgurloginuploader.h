#ifndef IMGURLOGINUPLOADER_H
#define IMGURLOGINUPLOADER_H

/* TODO Re-enable me!
 *
 * Imgur uploader for logged in users has been disabled due to removal of WebKit
 * module from Qt.
 *
 * Possible solutions:
 * - This uploader should use WebEngine (Chromium) instead of WebKit.
 */
#if 0

#include "imguranonuploader.h"
#include <QWidget>
#include <QWebView>

namespace Ui {
class ImgurLoginUploader;
}

class ImgurLoginUploader : public ImgurAnonUploader
{
	Q_OBJECT

public:
	explicit ImgurLoginUploader(QWidget *parent, QSettings &settings);
	~ImgurLoginUploader();

	virtual void load();

	virtual bool init(int imageNumber);
	virtual QString uploadImage(QString filePath, QIODevice *image);

protected:
	bool checkToken();
	void authorize();

private slots:
	void pageFinished();

protected:
	virtual void setAuthorization(NetworkTransaction *tr);

	Ui::ImgurLoginUploader *ui;
	static const QString resPage;

	QString accessToken;
	QString refreshToken;
	QDateTime expires;

};

#endif // IMGURLOGINUPLOADER_H
#endif // 0
