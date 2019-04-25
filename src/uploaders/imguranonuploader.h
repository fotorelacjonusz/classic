#ifndef IMGURANONYUPLOADER_H
#define IMGURANONYUPLOADER_H

#include "abstractuploader.h"
#include <QWidget>
#include <QDateTime>


namespace Ui {
class ImgurAnonUploader;
}

class ImgurAnonUploader : public AbstractUploader
{
	Q_OBJECT

public:
	explicit ImgurAnonUploader(QWidget *parent, QSettings &settings);
	~ImgurAnonUploader();

	virtual void load();
	virtual bool init(int imageNumber);
	virtual QString uploadImage(QString filePath, QIODevice *image);
	virtual QString tosUrl() const;

protected:
	void updateCredits();
	bool checkCredits(int imageNumber, int extra = 0);
	virtual void setAuthorization(NetworkTransaction *tr);

	struct Credits
	{
		Credits();
		int userLimit, userRemaining, appLimit, appRemaining;
	} credits;

	QString albumId;

	Ui::ImgurAnonUploader *ui;

};

#endif // IMGURANONYUPLOADER_H
