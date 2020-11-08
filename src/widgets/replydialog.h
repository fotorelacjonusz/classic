#ifndef REPLYDIALOG_H
#define REPLYDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QTime>
#include <QWebEnginePage>
#include "progresscontainer.h"
#include "abstractimage.h"
#include "postwidget.h"

class SettingsDialog;
class AbstractUploader;
class QSettings;

namespace Ui {
class ReplyDialog;
}

class ReplyDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ReplyDialog(QSettings &settings, QList<AbstractImage *> imageList, const QString &header, const QString &footer, QWidget *parent);
	virtual ~ReplyDialog();

	int latestPostedImageNumber() const;
	QString threadId() const;
	QString threadTitle() const;

public slots:
	void forumPageLoaded(QString url);
	void forumThreadVisited(QString replyUrl);
	void forumReplySubmissionFailed();

public:
	Q_INVOKABLE bool isNextPostAvailable();
	Q_INVOKABLE QString obtainNextPost();

protected slots:
	void appendTable(QString cell0, QString cell1);
	void setVisible(bool visible);

	void upload();
	void accept();
	void reject();

private slots:
	void startTimer();
	void tick();

	void on_hideInfoButton_clicked();

private:
	Ui::ReplyDialog *ui;
	QSettings &settings;
	QWebEnginePage *frame;
	AbstractUploader *const uploader;

	ProgressContainer<AbstractImage> images;
	ProgressContainer<PostWidget> posts;
	typedef ProgressContainer<AbstractImage>::Item ImageItem;
	typedef ProgressContainer<PostWidget>::Item PostItem;

	QString m_threadId, m_threadTitle;
	QString userName;

	QTimer timer;
	PostItem *nextPost;
	QTime time;

	static const QString likePostId;
};

#endif // REPLYDIALOG_H
