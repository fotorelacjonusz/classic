#ifndef IMGURANONYUPLOADER_H
#define IMGURANONYUPLOADER_H

#include "abstractuploader.h"
#include <QWidget>
#include <QDateTime>


namespace Ui {
class ImgurAnonyUploader;
}

class ImgurAnonyUploader : public AbstractUploader
{
	Q_OBJECT

public:
	explicit ImgurAnonyUploader(QWidget *parent, QSettings &settings);
	~ImgurAnonyUploader();

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

	Ui::ImgurAnonyUploader *ui;

};

#endif // IMGURANONYUPLOADER_H
