#include "replydialog.h"
#include "ui_replydialog.h"
#include "settings/settingsdialog.h"
#include "uploaders/abstractuploader.h"
#include "embeddedjavascript.h"
#include "networkcookiejar.h"

#include <QWebEnginePage>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkCookieJar>
#include <QNetworkDiskCache>
#include <QDesktopServices>
#include <QBuffer>
#include <QWebChannel>
#include <QtCore/qmath.h>

#define ALL_IMAGES_PROGRESS_MULTIPLIER 10000
#define DONT_SHOW_FORUM_INFO "dont_show_forum_info"

const QString ReplyDialog::likePostId = "94354890"; // fotorel
//const QString ReplyDialog::likePostId = "102485895"; // b

ReplyDialog::ReplyDialog(QSettings &settings, QList<AbstractImage *> imageList, const QString &header, const QString &footer, QWidget *parent):
	QDialog(parent),
	ui(new Ui::ReplyDialog),
	settings(settings),
	uploader(SETTINGS->uploader),
	delegate(&ReplyDialog::parseThread),
	nextPost(0)
{
	ui->setupUi(this);
	ui->toolBox->removeItem(0);

	images.setProgressBar(ui->progressBarAllImages);
	images.setFormat(tr("Wszystkie obrazki: %p%"));
	posts.setProgressBar(ui->progressBar);
	posts.setFormat(SETTINGS->isSelectedThread() ? tr("Przechodzę do wątku...") : tr("Czekam na wybranie wątku..."));

	foreach (AbstractImage *image, imageList)
		images.append(image, 1.0, ui->progressBarImage);
	for (int i = 0; i < qCeil((qreal)imageList.count() / SETTINGS->imagesPerPost); ++i)
		posts.append(new PostWidget(ui->toolBox), SETTINGS->postSpace);

	posts.first()->setTotal(0);
	posts.last()->object()->setLast();
	posts.setExtraTotal(posts.size()); // 1 navigation per post

	posts.first()->object()->setHeader(header);
	posts.last()->object()->setFooter(footer);

	if (settings.value(DONT_SHOW_FORUM_INFO).toBool())
		ui->infoWidget->hide();

#ifdef Q_OS_WIN
	ui->progressBar->setStyleSheet("QProgressBar { color: black; }");
	ui->progressBarImage->setStyleSheet("QProgressBar { color: black; }");
	ui->progressBarAllImages->setStyleSheet("QProgressBar { color: black; }");
#endif

	ui->tableWidget->setColumnWidth(0, 200);
	ui->tableWidget->setColumnWidth(1, 500);

	timer.setInterval(50);
	connect(&timer, SIGNAL(timeout()), this, SLOT(tick()));

	connect(ui->webView, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));

	frame = ui->webView->page();

	frame->setWebChannel(&webChannel, EmbeddedJavascript::worldId);
	webChannel.registerObject("replyDialog", this);

	QNetworkDiskCache *cache = new QNetworkDiskCache();
	cache->setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
}

ReplyDialog::~ReplyDialog()
{
	delete ui;
}

int ReplyDialog::latestPostedImageNumber() const
{
	PostItem *last = posts.last(PostWidget::Posted);
	return last ? last->object()->lastImageNumber() : -1;
}

QString ReplyDialog::threadId() const
{
	return m_threadId;
}

QString ReplyDialog::threadTitle() const
{
	return m_threadTitle;
}

void ReplyDialog::forumPageLoaded(QString url)
{
	qDebug() << "Browser on page" << url;
}

void ReplyDialog::appendTable(QString cell0, QString cell1)
{
	int row = ui->tableWidget->rowCount();
	ui->tableWidget->setRowCount(row + 1);
	ui->tableWidget->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));

	QTableWidgetItem *item0 = new QTableWidgetItem(cell0);
	QTableWidgetItem *item1 = new QTableWidgetItem(cell1);
	item0->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item1->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	ui->tableWidget->setItem(row, 0, item0);
	ui->tableWidget->setItem(row, 1, item1);
}

void ReplyDialog::setVisible(bool visible)
{
	QDialog::setVisible(visible);
	if (visible)
		QTimer::singleShot(0, this, SLOT(upload()));
}

void ReplyDialog::upload()
{
	delegate = &ReplyDialog::parseThread;
	ui->webView->load(SETTINGS->homeUrl);

	if (!uploader->init(images.count()))
	{
		reject();
		QMessageBox::critical(this, tr("Błąd"), tr("Nie można było rozpocząć wysyłania z powodu:\n%1").arg(uploader->lastError()));
		return;
	}

	for (ImageItem *item; (item = images.first(AbstractImage::Ready));)
	{
		QString fileName = item->object()->fileName();

		if (!item->object()->url().isEmpty())
		{
			item->setFormat(tr("Zdjęcie %1 już wysłane").arg(fileName));
			item->setProgress(1.0);
		}
		else
		{
			//		qDebug() << "wysyłam" << fileName;
			item->setFormat(tr("Wysyłam %1: %p%").arg(fileName));

			connect(uploader, SIGNAL(uploadProgress(qint64,qint64)), item, SLOT(setProgressScaleToOne(qint64,qint64)));
			QBuffer buffer;
			buffer.open(QIODevice::ReadWrite);
			item->object()->write(&buffer);
			buffer.seek(0);
			const QString url = uploader->uploadImage(fileName, &buffer);
			buffer.close();
			disconnect(uploader, SIGNAL(uploadProgress(qint64,qint64)), 0, 0);

			if (url.isEmpty())
			{
				reject();
				QMessageBox::critical(this, tr("Błąd"), tr("Nie można było wysłać obrazka %1 z powodu:\n%2").arg(fileName).arg(uploader->lastError()));
				return;
			}

			item->object()->setUrl(url);
		}
		item->state = AbstractImage::Uploaded;

		appendTable(fileName, item->object()->url());

		PostItem *post = posts.first(PostWidget::Incomplete);
		Q_ASSERT(post);
		post->object()->appendImage(item->object());
		item->state = AbstractImage::Assigned;
		if (post->object()->isFull() || images.all(AbstractImage::Assigned))
			post->state = PostWidget::Full;

		loadProgress(100);
	}

	uploader->finalize();
}

void ReplyDialog::accept()
{
	done(Accepted);
}

void ReplyDialog::reject()
{
	if (!delegate) // reject has already been called
		return;
	delegate = 0;
	uploader->abort();
	ui->webView->stop();
//	timer.stop();
	done(Rejected);
}

// Deliberately to remove.
#if 0
bool ReplyDialog::isElement(QString query, QString *variable, int up, QString attr) const
{
	if (variable && !variable->isEmpty())
		return true;
	QWebElement element = frame->findFirstElement(query);
	if (element.isNull())
		return false;
	for (int i = 0; i < up; ++i)
		element = element.parent();
	if (variable)
		*variable = attr.isEmpty() ? element.toPlainText() : element.attribute(attr);
	qDebug() << "jest element" << (variable ? *variable : 0);
	return true;
}

bool ReplyDialog::isElementRemove(QString query, QString *variable, QString pattern, QString attr) const
{
	return isElement(query, variable, 0, attr) && !(*variable = variable->remove(QRegExp(pattern)).trimmed()).isEmpty();
}
#endif

void ReplyDialog::startTimer()
{
	time.restart();
	timer.start();
}

void ReplyDialog::tick()
{
	if (nextPost->isProgressComplete())
	{
		timer.stop();
		loadProgress(0);
		return;
	}

	qreal elapsed = time.elapsed() / 1000.0;
	nextPost->setProgress(elapsed);
	int second = qCeil(nextPost->total() - elapsed);
	static const QStringList secondStrings = QStringList() << tr("sekund", "0") << tr("sekundę", "1") << tr("sekundy", "2")
														   << tr("sekundy", "3") << tr("sekundy", "4") << tr("sekund", "5");
	posts.setFormat(tr("Czekam %1 %2... %p%").arg(second).arg(secondStrings[qBound(0, second, 5)]));

//	qDebug() << "tick()       " << posts.progress() << posts.total();
}

void ReplyDialog::loadProgress(int progress)
{
	if (delegate)
		(this->*delegate)(progress);
}

/**
 * @deprecated This method was not removed yet only because its name is
 * still referenced elsewhere.
 * @todo To be removed.
 */
void ReplyDialog::parseThread(int progress)
{}

/**
 * @brief Handles reply submission failure.
 *
 * This typically happens due to throttling.  Forum requires some wait between
 * posts.
 *
 * Original application used to extract a message displayed to user in order to
 * determine required wait time.  However, it could break if they change message
 * format or user switches to localized interface.  This is why the wait time
 * has been hardcoded.
 *
 * Extracted from https://github.com/skalee/fotorelacjonusz/blob/4aefec4e3086ee2c2682ad0905073f1da63f1727/src/widgets/replydialog.cpp#L257-L278.
 *
 * @todo Old method structure has been preserved for better git diffs.
 * Further clean-ups may be needed.
 */
void ReplyDialog::forumReplySubmissionFailed()
{
	PostItem *sentPost = posts.first(PostWidget::Sent);
	if (sentPost)
	{
		if (true)
		{
			// rollback
			delegate = &ReplyDialog::sendPost;
			// TODO Constantize it
			int secs = 10; // Hardcode 10 secs wait
			qDebug() << "Throttled, waiting" << secs << "seconds";

			sentPost->state = PostWidget::Full; // -1
			sentPost->setTotal(secs);
			sentPost->setProgress(0);
			startTimer();
			return;
		}
	}
}

/**
 * @brief Handles visit on show thread page.
 *
 * Determines whether there are any posts left, and opens reply page if so.
 * Ends submission process otherwise.
 *
 * @param QString link to reply page.
 *
 * @todo Old method structure has been preserved for better git diffs.
 * Further clean-ups may be needed.
 */
void ReplyDialog::forumThreadVisited(QString replyLink)
{
	qDebug() << "Visited thread page!";

	PostItem *sentPost = posts.first(PostWidget::Sent);

	// confirm post, this must be done after extracting imgReply
	if (sentPost)
	{
		sentPost->state = PostWidget::Posted;
		posts.increaseExtraProgress(1.0);
	}

	if (posts.all(PostWidget::Posted))
	{
		delegate = 0;
		qDebug() << "parseThread()" << "koniec\n";
		posts.setFormat(tr("Koniec. %p%"));
		ui->webView->setEnabled(true);
		ui->buttonBox->clear();
		ui->buttonBox->addButton(QDialogButtonBox::Ok);
		return;
	}

	delegate = &ReplyDialog::sendPost;
	ui->webView->setEnabled(false);
	ui->webView->stop();

	nextPost = posts.firstAtBest(PostWidget::Full);
	if (nextPost)
	{
		startTimer();
		qDebug() << "parseThread()" << "odpalam timer";
	}

	qDebug() << "parseThread()" << "przechodzę do formularza\n";
	posts.setFormat(tr("Przechodzę do formularza... %p%"));
	ui->webView->load(replyLink);
}

/**
 * @deprecated This method was not removed yet only because its name is
 * still referenced elsewhere.
 * @todo To be removed.
 */
void ReplyDialog::sendPost(int progress)
{}

/**
 * @brief Checks if obtainNextPost() can be called.
 * @retval true At least one post is ready to be sent to forum (i.e. photos have
 * been uploaded, and links to them are available).
 * @retval false Otherwise.
 *
 * @todo Old method structure has been preserved for better git diffs.
 * Further clean-ups may be needed.
 */
bool ReplyDialog::isNextPostAvailable()
{
	PostItem *post = posts.firstAtBest(PostWidget::Full); // first Incomplete or Full

	if (!post) // no posts
	{
		qDebug() << "sendPost()   " << "nie ma postów\n";
		return false;
	}

	if (post->state != PostWidget::Full) // Incomplete - post not yet filled with all images
	{
		qDebug() << "sendPost()   " << "post niegotowy\n";
		return false;
	}

	if (!post->isProgressComplete()) // post time not passed
	{
		qDebug() << "sendPost()   " << "timer niezakończony\n";
		return false;
	}

	return true;
}

/**
 * @brief Obtains text of the next reply to be posted, and marks that post as
 * sent.
 * @note Undefined behaviour unless there is any complete post of which body
 * can be returned.
 * @return QString containing BBCode which is about to be posted.
 *
 * @todo Old method structure has been preserved for better git diffs.
 * Further clean-ups may be needed.
 */
QString ReplyDialog::obtainNextPost()
{
	PostItem *post = posts.firstAtBest(PostWidget::Full); // first Incomplete or Full
	QString postBody = post->object()->text();
	post->state = PostWidget::Sent; // Full -> Sent
	qDebug() << "sendPost()   " << "wysyłam posta\n";
	delegate = &ReplyDialog::parseThread;
	posts.setFormat(tr("Wysyłam posta... %p%"));

	return postBody;
}

void ReplyDialog::on_hideInfoButton_clicked()
{
	if (ui->dontShowAgain->isChecked())
		settings.setValue(DONT_SHOW_FORUM_INFO, true);
	ui->infoWidget->hide();
}
