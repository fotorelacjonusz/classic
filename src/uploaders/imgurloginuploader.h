#pragma once

#include "imguranonuploader.h"
#include "imgurauthenticator.h"
#include <QWidget>

namespace Ui {
class ImgurLoginUploader;
}

/**
 * @brief Handles authenticated uploads to Imgur.
 *
 * This class is responsible for uploader configuration as well as very uploads.
 * This class inerits a lot of commmon behaviour from ImgurAnonUploader.
 *
 * Uploaded pictures are coupled with user's account, and aggregated in
 * an album.
 */
class ImgurLoginUploader : public ImgurAnonUploader
{
	Q_OBJECT

	typedef ImgurAuthenticator::Credentials Credentials;

public:
	explicit ImgurLoginUploader(QWidget *parent, QSettings &settings);
	~ImgurLoginUploader();

	virtual void load();

	virtual bool init(int imageNumber);
	virtual QString uploadImage(QString filePath, QIODevice *image);

	void refreshAuthorization();

	void updateLoginInfo();

public slots:
	void requestAuthorization();
	void finalizeAuthorization(Credentials);

protected:
	/// Checks if credentials are present and valid.
	bool checkToken();

protected:
	virtual void setAuthorization(NetworkTransaction *tr);

	ImgurAuthenticator auth;
	Credentials credentials;

	Ui::ImgurLoginUploader *ui;
};

