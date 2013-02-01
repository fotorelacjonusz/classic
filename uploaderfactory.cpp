#include "uploaderfactory.h"
#include "abstractuploader.h"

#include "isanonyuploader.h"
#include "iscodeuploader.h"
#include "isloginuploader.h"
#include "ftpuploader.h"
#include "imguranonyuploader.h"
#include "imgurloginuploader.h"

UploaderFactory::AbstarctWrapper::AbstarctWrapper(QString name):
	name(name)
{
}

UploaderFactory::AbstarctWrapper::~AbstarctWrapper()
{
}

template <class T>
UploaderFactory::Wrapper<T>::Wrapper(QString name):
	AbstarctWrapper(name)
{
}

template <class T>
UploaderFactory::Wrapper<T>::~Wrapper()
{
	if (!object.isNull())
		delete object;
}

template <class T>
AbstractUploader *UploaderFactory::Wrapper<T>::make(QWidget *w, QSettings &s)
{
	return !object.isNull() ? object : object = new T(w, s);
}

#define UPLOADER(T, S) uploaders.append(new Wrapper<T>(S));

UploaderFactory::UploaderFactory()
{
	UPLOADER(IsAnonyUploader, tr("Imageshack anonimowo"));
	UPLOADER(IsCodeUploader, tr("Imageshack kod rejestracyjny"));
	UPLOADER(IsLoginUploader, tr("Imageshack login i hasło"));
	UPLOADER(FtpUploader, tr("Własne konto FTP"));
	UPLOADER(ImgurAnonyUploader, tr("Imgur anonimowo"));
	UPLOADER(ImgurLoginUploader, tr("Imgur konto"));
}

UploaderFactory::~UploaderFactory()
{
	qDeleteAll(uploaders);
}







