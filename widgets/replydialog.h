#ifndef REPLYDIALOG_H
#define REPLYDIALOG_H

#include <QDialog>
#include <QTimer>

class QWebFrame;
class SettingsDialog;
class AbstractImage;
class PostWidget;
class AbstractUploader;
class QSettings;

namespace Ui {
class ReplyDialog;
}

class ReplyDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit ReplyDialog(QSettings &settings, QList<AbstractImage *> images, QWidget *parent);
	virtual ~ReplyDialog();
	
	int getLatestPostedImageNumber() const;
	QString getThreadId() const;
	QString getThreadTitle() const;
	
public slots:
	void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
	
//signals:
//	void imagePosted(QString threadId, QString threadTitle, int imageNumber);

protected:
	void appendTable(QString cell0, QString cell1);

	void setVisible(bool visible);
	
protected slots:
	void upload();
	void accept();
	void reject();
	
private:
	bool isElement(QString query, QString *variable = 0, int up = 0, QString attr = QString()) const;
	bool isElementRemove(QString query, QString *variable, QString pattern, QString attr = QString()) const;

private slots:
	void tick();
	void loadProgress(int progress = 0);

private:
	void parseThread(int progress);
	void sendPost(int progress);

private slots:	
	void likeClicked();
	void on_hideInfoButton_clicked();
	
private:
	void searchLikeEntry(int progress);
	void likeProgress(int progress);
	void increaseProgress(int progress);

	Ui::ReplyDialog *ui;
	QSettings &settings;
	QWebFrame *frame;
	AbstractUploader *const uploader;
	QList<AbstractImage *> images;
	QList<PostWidget *> posts, sentPosts;
	QPushButton *likeButton;
	QString threadId, threadTitle;
//	PostWidget *lastSentPost;
	int latestPostedImageNumber;
	QString userName;

	void (ReplyDialog::*delegate)(int);

	int imagesUploaded;
	QTimer timer;
	int timerCounter;
	const int fps;
	bool allImagesUploaded;
//	bool firstPostSent;

	static const QString likePostId;
};

#endif // REPLYDIALOG_H
