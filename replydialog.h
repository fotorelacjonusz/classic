#ifndef REPLYDIALOG_H
#define REPLYDIALOG_H

#include <QDialog>
#include <QEventLoop>
#include <QWebElement>
#include <QTimer>

class QWebFrame;
class SettingsDialog;
class AbstractImage;
class PostWidget;
class AbstractUploader;

namespace Ui {
class ReplyDialog;
}

class ReplyDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit ReplyDialog(QList<AbstractImage *> images, QWidget *parent);
	virtual ~ReplyDialog();
	
public slots:
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);

protected:
	void appendTable(QString cell0, QString cell1);

	void setVisible(bool visible);
	void reject();

private slots:
	void tick();
	void likeClicked();
	void loadProgress(int progress);

private:
	void parseThread(int progress);
	void sendPost(int progress);
	void likeProgress(int progress);
	void increaseProgress(int progress);

	Ui::ReplyDialog *ui;
	QWebFrame *frame;
	AbstractUploader *const uploader;
	QList<AbstractImage *> images;
	QList<PostWidget *> posts;
	QPushButton *likeButton;

	void (ReplyDialog::*delegate)(int);

	int imagesUploaded;
	QTimer timer;
	int timerCounter;
	const int fps;
	bool allImagesUploaded;
	bool firstPostSent;

	static const QString postId;
};

#endif // REPLYDIALOG_H
