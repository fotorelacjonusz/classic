#pragma once

#include "imguranonuploader.h"
#include "imgurauthenticator.h"
#include <QWidget>

namespace Ui {
class ImgurLoginUploader;
}

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
	bool checkToken();

protected:
	virtual void setAuthorization(NetworkTransaction *tr);

	ImgurAuthenticator auth;
	Credentials credentials;

	Ui::ImgurLoginUploader *ui;
};

