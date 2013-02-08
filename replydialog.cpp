#include "replydialog.h"
#include "ui_replydialog.h"
#include "settingsdialog.h"
#include "abstractimage.h"
#include "postwidget.h"
#include "abstractuploader.h"
#include "networkcookiejar.h"

#include <QWebFrame>
#include <QWebElement>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkCookieJar>
#include <QtCore/qmath.h>

#define ALL_IMAGES_PROGRESS_MULTIPLIER 10000

const QString ReplyDialog::postId = "94354890"; // fotorel

ReplyDialog::ReplyDialog(QList<AbstractImage *> images, QWidget *parent):
	QDialog(parent),
	ui(new Ui::ReplyDialog),
	uploader(SETTINGS->uploader),
	images(images),
	likeButton(0),
	delegate(&ReplyDialog::parseThread),
	imagesUploaded(0),
	timerCounter(0),
	fps(10),
	allImagesUploaded(false),
	firstPostSent(false)
{
	int postCount = qCeil(qreal(images.count()) / SETTINGS->imagesPerPost);
	ui->setupUi(this);
	ui->toolBox->removeItem(0);
	ui->progressBarAllImages->setMaximum(images.count() * ALL_IMAGES_PROGRESS_MULTIPLIER);
	ui->progressBarAllImages->setFormat(tr("Wszystkie obrazki: %p%"));
	ui->progressBar->setMaximum(((postCount - 1) * SETTINGS->postSpace + 2 * postCount + 1) * fps);
	ui->progressBar->setFormat(tr("Czekam na wybranie wątku..."));
	// postSpace seconds between posts plus 2 navigations per post
	// (reply form, thread) plus 1 navigation in the end (thread). I assumed one navigation takes 1 second.
	// Po postSpace sekund odstępu między postami, plus na każdy post po 2 ładowania strony
	// (formularz odpowiedzi, wątek) plus na koniec jedno (wątek). Ładowanie ~= 1 sekunda.

	ui->tableWidget->setColumnWidth(0, 200);
	ui->tableWidget->setColumnWidth(1, 500);

	timer.setSingleShot(true);
	timer.setInterval(1000 / fps);
	connect(&timer, SIGNAL(timeout()), this, SLOT(tick()));

	QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, false);
	connect(ui->webView, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
	connect(uploader, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(uploadProgress(qint64,qint64)));


//	ui->webView->setPage(new WebPage());
	frame = ui->webView->page()->mainFrame();
	ui->webView->page()->networkAccessManager()->setCookieJar(new NetworkCookieJar());
	ui->webView->load(SETTINGS->homeUrl);

//	likeButton = ui->buttonBox->addButton(tr("Lubię to"), QDialogButtonBox::ActionRole);
//	connect(likeButton, SIGNAL(clicked()), this, SLOT(likeClicked()));
}

ReplyDialog::~ReplyDialog()
{
	delete ui;
}

void ReplyDialog::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
	ui->progressBarImage->setMaximum(bytesTotal);
	ui->progressBarImage->setValue(bytesSent);
	ui->progressBarAllImages->setValue(((qreal(bytesSent) / bytesTotal) + imagesUploaded) * ALL_IMAGES_PROGRESS_MULTIPLIER);
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
	{
		if (!uploader->init(images.count()))
		{
			reject();
			QMessageBox::critical(this, tr("Błąd"), tr("Nie można było rozpocząć wysyłania z powodu:\n%1").arg(uploader->lastError()));
			return;
		}

		QString openingTags = SETTINGS->extraTags.value().remove('\n');
		QString closingTags;
		QRegExp tagExp("\\[([^=\\]]+)(=.+)?\\]");
		tagExp.setMinimal(true);
		for (int pos = 0; (pos = tagExp.indexIn(openingTags, pos)) != -1; pos += tagExp.matchedLength())
			closingTags.prepend("[/" + tagExp.cap(1) + "]");

		for (int postNumber = 0; !images.isEmpty(); ++postNumber)
		{
			PostWidget *post = new PostWidget(ui->toolBox);
			connect(post, SIGNAL(appended(int)), this, SLOT(loadProgress(int)));
			ui->toolBox->addItem(post, tr("Post %1").arg(postNumber + 1));
			posts << post;
			post->append(openingTags);

			for (int i = 0; i < SETTINGS->imagesPerPost && !images.isEmpty(); ++i)
			{
				AbstractImage *image = images.takeFirst();
				ui->progressBarImage->reset();
				ui->progressBarImage->setFormat(tr("Wysyłam %1: %p%").arg(image->getFileName()));
				if (!image->upload(uploader))
				{
					reject();
					QMessageBox::critical(this, tr("Błąd"), tr("Nie można było wysłać obrazka %1 z powodu:\n%2").arg(image->getFileName()).arg(uploader->lastError()));
					return;
				}

				ui->progressBarAllImages->setValue(++imagesUploaded * ALL_IMAGES_PROGRESS_MULTIPLIER);

				appendTable(image->getFileName(), image->getUrl());
				post->append(image->toForumCode());

			}

			if (SETTINGS->addTBC && !images.isEmpty())
				post->append(tr("Cdn ..."));

			post->append(closingTags, true);
		}

		uploader->finalize();
		allImagesUploaded = true;
	}
}

void ReplyDialog::reject()
{
	images.clear();
	posts.clear();
	uploader->abort();
	ui->webView->stop();
	timer.stop();
	QDialog::reject();
}

void ReplyDialog::tick()
{
	--timerCounter;
	increaseProgress(1);
	int second = qCeil(qreal(timerCounter) / fps);
	static const QStringList secondStrings = QStringList() << tr("sekund", "0") << tr("sekundę", "1") << tr("sekundy", "2")
														   << tr("sekundy", "3") << tr("sekundy", "4") << tr("sekund", "5");
	ui->progressBar->setFormat(tr("Czekam %1 %2... %p%").arg(second).arg(second < secondStrings.size() ? secondStrings[second] : secondStrings.last()));
	if (timerCounter > 0)
		timer.start();
	else
		loadProgress(0);
}

void ReplyDialog::likeClicked()
{
	qDebug() << "like clicked";
	delegate = &ReplyDialog::likeProgress;
	likeButton->setEnabled(false);
	QUrl url = "http://www.skyscrapercity.com/showthread.php?p=" + postId;
	if (ui->webView->url() != url)
	{
		ui->webView->setEnabled(false);
		ui->webView->stop();
		ui->webView->load(url);
	}
	else
		likeProgress(100);
}

void ReplyDialog::loadProgress(int progress)
{
	if (delegate)
		(this->*delegate)(progress);
}

void ReplyDialog::parseThread(int progress)
{
	Q_UNUSED(progress);
	QWebElement imgReply = frame->findFirstElement("html > body > center > div > div.page > div > table > tbody > tr > td > a > img[alt=Reply]");
	if (imgReply.isNull())
		return;

	qDebug() << "parseThread()" << "jest reply button";
	delegate = &ReplyDialog::sendPost;
	increaseProgress(fps);
	ui->webView->setEnabled(false);
	ui->webView->stop();

	if (allImagesUploaded && posts.isEmpty())
	{
		qDebug() << "parseThread()" << "koniec\n";
		ui->progressBar->setFormat(tr("Koniec. %p%"));
		ui->webView->setEnabled(true);
		ui->buttonBox->clear();
		ui->buttonBox->addButton(QDialogButtonBox::Ok);
		likeButton = ui->buttonBox->addButton(tr("Lubię to"), QDialogButtonBox::ActionRole);
		connect(likeButton, SIGNAL(clicked()), this, SLOT(likeClicked()));
		return;
	}

	if (firstPostSent)
	{
		qDebug() << "parseThread()" << "odpalam timer";
		timerCounter = SETTINGS->postSpace * fps;
		timer.start();
	}

	qDebug() << "parseThread()" << "przechodzę do formularza\n";
	QString url = QString(SSC_HOST) + "/" + imgReply.parent().attribute("href");
	ui->progressBar->setFormat(tr("Przechodzę do formularza... %p%"));
	ui->webView->load(url);
}

void ReplyDialog::sendPost(int progress)
{
	Q_UNUSED(progress);
	QWebElement title = frame->findFirstElement("input[name=title]");
	QWebElement message = frame->findFirstElement("textarea[name=message]");
	QWebElement submit = frame->findFirstElement("input[name=sbutton]");
	if (title.isNull() || message.isNull() || submit.isNull())
		return;

	qDebug() << "sendPost()   " << "jest formularz";
	QWebElement href = frame->findFirstElement("form[name=vbform] > table.tborder > tbody > tr > td.tcat > span.smallfont > a");
	href.setAttribute("name", "scrollTo");
	frame->scrollToAnchor("scrollTo");

	if (posts.isEmpty())
	{
		qDebug() << "sendPost()   " << "nie ma postów\n";
		return;
	}

	message.setPlainText(posts.first()->text());
	if (!posts.first()->isReady())
	{
		qDebug() << "sendPost()   " << "post niegotowy\n";
		return;
	}

	if (timerCounter > 0)
	{
		qDebug() << "sendPost()   " << "timer niezakończony\n";
		return;
	}

	qDebug() << "sendPost()   " << "wysyłam posta\n";
	delegate = &ReplyDialog::parseThread;
	increaseProgress(fps);
	posts.removeFirst();
	firstPostSent = true;
	ui->progressBar->setFormat(tr("Wysyłam posta... %p%"));
	submit.evaluateJavaScript("this.click()");
}

void ReplyDialog::likeProgress(int progress)
{
	if (progress == 100)
	{
		QWebElement likeLink = frame->findFirstElement(QString("a[data-postid=\"%1\"]").arg(postId));
//		if (!likeLink.isNull())
		likeLink.evaluateJavaScript("var evObj = document.createEvent('MouseEvents'); evObj.initEvent('click', true, true); this.dispatchEvent(evObj);");

		qDebug() << "likeProgress(100) likeLink:" << !likeLink.isNull();
		likeButton->setEnabled(true);
		ui->webView->setEnabled(true);
		delegate = 0;
	}
}

void ReplyDialog::increaseProgress(int progress)
{
	ui->progressBar->setValue(ui->progressBar->value() + progress);
}
