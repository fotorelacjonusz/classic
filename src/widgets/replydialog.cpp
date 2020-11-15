#include "replydialog.h"
#include "ui_replydialog.h"
#include "settings/settingsdialog.h"
#include "uploaders/abstractuploader.h"

#include <QDebug>
#include <QMessageBox>
#include <QBuffer>
#include <QtMath>

ReplyDialog::ReplyDialog(QSettings &settings, QList<AbstractImage *> imageList, const QString &header, const QString &footer, QWidget *parent):
	QDialog(parent),
	ui(new Ui::ReplyDialog),
	settings(settings),
	uploader(SETTINGS->uploader),
	nextPost(nullptr)
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
		posts.append(new PostWidget(ui->toolBox));

	posts.first()->setTotal(0);
	posts.last()->object()->setLast();
	posts.setExtraTotal(posts.size()); // 1 navigation per post

	posts.first()->object()->setHeader(header);
	posts.last()->object()->setFooter(footer);

#ifdef Q_OS_WIN
	ui->progressBar->setStyleSheet("QProgressBar { color: black; }");
	ui->progressBarImage->setStyleSheet("QProgressBar { color: black; }");
	ui->progressBarAllImages->setStyleSheet("QProgressBar { color: black; }");
#endif

	ui->tableWidget->setColumnWidth(0, 200);
	ui->tableWidget->setColumnWidth(1, 500);

	timer.setInterval(50);
	connect(&timer, SIGNAL(timeout()), this, SLOT(tick()));
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
			disconnect(uploader, SIGNAL(uploadProgress(qint64,qint64)), nullptr, nullptr);

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
	}

	uploader->finalize();

	this->finishSubmission();
}

void ReplyDialog::accept()
{
	done(Accepted);
}

void ReplyDialog::reject()
{
	uploader->abort();
//	timer.stop();
	done(Rejected);
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


/**
 * @brief Changes UI when submission is done.
 */
void ReplyDialog::finishSubmission()
{
	qDebug() << "parseThread()" << "koniec\n";
	posts.setFormat(tr("Koniec. %p%"));
	ui->buttonBox->clear();
	ui->buttonBox->addButton(QDialogButtonBox::Ok);
	return;
}
