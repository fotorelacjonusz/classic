#include "ftpuploader.h"
#include "ui_ftpuploader.h"
#include <QSettings>
#include <QDir>

FtpUploader::FtpUploader(QWidget *parent, QSettings &settings) :
	AbstractUploader(parent, settings),
	ui(new Ui::FtpUploader)
{
	ui->setupUi(this);
	connect(&ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SIGNAL(uploadProgress(qint64,qint64)));
	connect(&ftp, SIGNAL(commandFinished(int,bool)), &loop, SLOT(quit()));

	manager.makeInput("ftp/host", ui->hostEdit);
	manager.makeInput("ftp/port", ui->portSpinBox, 21);
	manager.makeInput("ftp/login", ui->loginEdit);
	manager.makeInput("ftp/password", ui->passwordEdit);
	manager.makeInput("ftp/url", ui->urlEdit);
}

FtpUploader::~FtpUploader()
{
	delete ui;
}

bool FtpUploader::init(int imageNumber)
{
	Q_UNUSED(imageNumber);
	finalize();
	QString password = ui->passwordEdit->text();
	if (password.isEmpty())
		password = queryPassword(tr("%1 na %2").arg(ui->loginEdit->text()).arg(ui->hostEdit->text()));
	if (password.isEmpty())
		return false;

	ftp.connectToHost(ui->hostEdit->text(), ui->portSpinBox->value());
	if (!advance())
		return false;
	ftp.login(ui->loginEdit->text(), password);
	if (!advance())
	{
		tempPassword.clear();
		return false;
	}

	dirName = QDateTime::currentDateTime().toString(Qt::ISODate);
	ftp.mkdir(dirName);
	if (!advance())
		return false;
	ftp.cd(dirName);
	if (!advance())
		return false;
	return true;
}

QString FtpUploader::uploadImage(QString filePath, QIODevice *image)
{
	QString fileName = filePath.split(QDir::separator()).last();

	image->open(QIODevice::ReadOnly);
	ftp.put(image, fileName);
	if (!advance())
		return "";
	image->close();

	return ui->urlEdit->text() + "/" + dirName + "/" + fileName;
}

void FtpUploader::abort()
{
	ftp.abort();
}

void FtpUploader::finalize()
{
	ftp.close();
	advance();
}

bool FtpUploader::advance()
{
	loop.exec();
	if (ftp.error() != QFtp::NoError)
	{
		error = ftp.errorString();
		return false;
	}
	return true;
}
