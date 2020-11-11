#include "uploaderfactory.h"

#include "isanonuploader.h"
#include "iscodeuploader.h"
#include "isloginuploader.h"
#include "ftpuploader.h"
#include "imguranonuploader.h"
#include "imgurloginuploader.h"

UploaderFactory::AbstractWrapper::AbstractWrapper(QString name):
	name(name)
{
}

UploaderFactory::AbstractWrapper::~AbstractWrapper()
{
}

template <class T>
class Wrapper : public UploaderFactory::AbstractWrapper
{
public:
	Wrapper(QString name):
		AbstractWrapper(name)
	{}

	virtual ~Wrapper()
	{
		delete object;
	}

	AbstractUploader *make(QWidget *w, QSettings &s)
	{
		return !object.isNull() ? object : object = new T(w, s);
	}

private:
	QPointer<T> object;
};

#define UPLOADER(T, S) uploaders.append(new Wrapper<T>(S));

UploaderFactory::UploaderFactory()
{
//	TODO Re-enable uploaders!  (See respective header files)

	UPLOADER(ImgurAnonUploader, tr("Imgur anonimowo"));
	UPLOADER(ImgurLoginUploader, tr("Imgur konto"));
//	UPLOADER(IsAnonUploader, tr("Imageshack anonimowo"));
//	UPLOADER(IsCodeUploader, tr("Imageshack kod rejestracyjny"));
//	UPLOADER(IsLoginUploader, tr("Imageshack login i hasło"));
	UPLOADER(FtpUploader, tr("Własne konto FTP"));
}

UploaderFactory::~UploaderFactory()
{
	qDeleteAll(uploaders);
}
