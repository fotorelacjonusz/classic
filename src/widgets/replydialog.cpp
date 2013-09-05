#include "replydialog.h"
#include "ui_replydialog.h"
#include "settings/settingsdialog.h"
#include "uploaders/abstractuploader.h"
#include "networkcookiejar.h"

#include <QWebFrame>
#include <QWebElement>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkCookieJar>
#include <QNetworkDiskCache>
#include <QDesktopServices>
#include <QBuffer>
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
	likeButton(0),
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

	QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, false);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true);
	connect(ui->webView, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));

	frame = ui->webView->page()->mainFrame();
	QNetworkDiskCache *cache = new QNetworkDiskCache();
	cache->setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
	ui->webView->page()->networkAccessManager()->setCache(cache);
	ui->webView->page()->networkAccessManager()->setCookieJar(new NetworkCookieJar());

//	likeButton = ui->buttonBox->addButton(tr("Lubię ten program"), QDialogButtonBox::ActionRole);
//	connect(likeButton, SIGNAL(clicked()), this, SLOT(likeClicked()));
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

void ReplyDialog::parseThread(int progress)
{
	Q_UNUSED(progress);
//	qDebug() << "parseThread()" << ui->webView->url() << progress;	
//	qDebug() << "parseThread()" << posts.progress() << posts.total();
	
	PostItem *sentPost = posts.first(PostWidget::Sent);
	if (sentPost && ui->webView->url().path().startsWith("/newreply.php"))
	{
		// error occurred - too early?
		QRegExp errorExp("Please try again in ([0-9]+) seconds.");		
		QWebElement errorElement = frame->findFirstElement("html > body > center > div > div.page > div > script + table > tbody > tr + tr > td > ol > li");
		qDebug() << "error element" << errorElement.isNull();
		if (!errorElement.isNull() && errorElement.toPlainText().contains(errorExp))
		{
			// rollback
			delegate = &ReplyDialog::sendPost;
			int secs = errorExp.cap(1).toInt();
			qDebug() << "parseThread()" << "za wcześnie o" << secs << "sekund";
			
			sentPost->state = PostWidget::Full; // -1
			sentPost->setTotal(secs);
			sentPost->setProgress(0);
			errorElement.removeFromDocument();
			startTimer();
			return;
		}
	}
	
//	qDebug() << "parseThread()" << posts.progress() << posts.total();

	// extract userName
	if (!isElement("html > body > center > div > div.page > div > table.tborder > tbody > tr > td.alt2 > div.smallfont > strong > a", &userName))
		return;
	// extract imgReply button with link to reply, so we know we are ia a thread actually, not on the forum
	QString replyLink;
	if (!isElement("html > body > center > div > div.page > div > table > tbody > tr > td > a > img[alt=Reply]", &replyLink, 1, "href"))
		return;
	// extract thread title, this must be done after extracting imgReply
	if (!isElementRemove("html > head > title", &m_threadTitle, "(- Page [0-9]+ )?- SkyscraperCity"))
		return;
	// extract thread id, this must be done after extracting imgReply
	if (!isElementRemove("div#threadtools_menu img[alt=\"Subscription\"] + a", &m_threadId, "[^0-9]+", "href"))
		return;
	
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
		likeButton = ui->buttonBox->addButton(tr("Lubię ten program"), QDialogButtonBox::ActionRole);
		connect(likeButton, SIGNAL(clicked()), this, SLOT(likeClicked()));
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
	ui->webView->load(QString(SSC_HOST) + "/" + replyLink);
}

void ReplyDialog::sendPost(int progress)
{
	Q_UNUSED(progress);
	QWebElement title = frame->findFirstElement("input[name=title]");
	QWebElement message = frame->findFirstElement("textarea[name=message]");
	QWebElement submit = frame->findFirstElement("input[name=sbutton]");
	if (title.isNull() || message.isNull() || submit.isNull()) // page not loaded enough
		return;

	qDebug() << "sendPost()   " << "jest formularz";
	QWebElement href = frame->findFirstElement("form[name=vbform] > table.tborder > tbody > tr > td.tcat > span.smallfont > a");
	href.setAttribute("name", "scrollTo");
	frame->scrollToAnchor("scrollTo");

	PostItem *post = posts.firstAtBest(PostWidget::Full); // first Incomplete or Full
	if (!post) // no posts
	{
		qDebug() << "sendPost()   " << "nie ma postów\n";
		return;
	}

	message.setPlainText(post->object()->text());
	if (post->state != PostWidget::Full) // Incomplete - post not yet filled with all images
	{
		qDebug() << "sendPost()   " << "post niegotowy\n";
		return;
	}

	if (!post->isProgressComplete()) // post time not passed
	{
		qDebug() << "sendPost()   " << "timer niezakończony\n";
		return;
	}

	post->state = PostWidget::Sent; // Full -> Sent
	qDebug() << "sendPost()   " << "wysyłam posta\n";
	delegate = &ReplyDialog::parseThread;
	posts.setFormat(tr("Wysyłam posta... %p%"));
	submit.evaluateJavaScript("this.click()");
}

void ReplyDialog::likeClicked()
{
//	qDebug() << "like clicked";
	ui->webView->stop();
	delegate = &ReplyDialog::likeProgress;
	likeButton->setEnabled(false);

	QUrl url = "http://www.skyscrapercity.com/showthread.php?p=" + likePostId;
	QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, false);
	// if already loaded
	if (ui->webView->url() == url)
		likeProgress(100);
	else
	{
		ui->webView->setEnabled(false);
		ui->webView->stop();
		ui->webView->load(url);
	}
}

void ReplyDialog::likeProgress(int progress)
{
	qDebug() << progress;
	if (progress == 100)
	{
		QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true);
		ui->webView->setEnabled(true);
		delegate = 0;
		
		foreach (QWebElement href, frame->findFirstElement(QString("td[id=\"dbtech_thanks_entries_%1\"]").arg(likePostId)).findAll("a"))
			if (href.toPlainText() == userName)
			{
				QMessageBox::information(this, tr("Lubię ten program"), tr("Wygląda na to, że już polubiłeś program."));
				return;
			}
		
		QWebElement likeLink = frame->findFirstElement(QString("a[data-postid=\"%1\"]").arg(likePostId));
//		if (!likeLink.isNull())
		likeLink.evaluateJavaScript("var evObj = document.createEvent('MouseEvents'); evObj.initEvent('click', true, true); this.dispatchEvent(evObj);");

		qDebug() << "likeProgress(100) likeLink:" << !likeLink.isNull();
		likeButton->setEnabled(true);		
	}
}

void ReplyDialog::on_hideInfoButton_clicked()
{
	if (ui->dontShowAgain->isChecked())
		settings.setValue(DONT_SHOW_FORUM_INFO, true);
	ui->infoWidget->hide();
}
