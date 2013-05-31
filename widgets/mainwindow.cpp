#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imagewidget.h"
#include "abstractimage.h"
#include "arrowwidget.h"
#include "replydialog.h"
#include "downloaders/gpsdata.h"
#include "license.h"

#include <QUrl>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QDateTime>
#include <QShortcut>
#include <QScrollBar>

QByteArray MainWindow::phrFileHeader("PHR PHotoRelation file. Program info: http://www.skyscrapercity.com/showthread.php?t=1539539 Author: Kamil Ostaszewski ");

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	manager(settings),
	settingsDialog(this, settings),
	selectedImage(0),
	selectedArrow(0),
	recentThreads(settings, this)
{
	ui->setupUi(this);
	ui->imageToolBar->setEnabled(false);
	ui->arrowToolBar->setEnabled(false);
	ui->colorManipulationBar->setEnabled(false);
	ui->menu_Fotorelacja->insertMenu(ui->menu_Fotorelacja->actions()[2], &recentThreads);
	ui->commonMap->setFirstWidget(ui->header);
	settingsDialog.copyDescriptions(this);

	manager.makeInput("geometry", (QWidget *)this);
	manager.makeInput("window_state", (QMainWindow *)this);
	manager.makeInput("dir_name", &dirName);
	manager.load();

	connect(&gpsData, SIGNAL(mapReady(QImage)), this, SLOT(commonMapReady(QImage)));
	
	SETTINGS->addCommonMap.connect(this, SLOT(updateCommonMap()));
	
	connect(new SelectableWidget<ArrowWidget>::Listener(this), SIGNAL(selected(QWidget*)), this, SLOT(arrowWidgetSelected(QWidget*)));
	connect(new SelectableWidget<ImageWidget>::Listener(this), SIGNAL(selected(QWidget*)), this, SLOT(imageWidgetSelected(QWidget*)));
	
	connect(ui->header, SIGNAL(focusIn()), this, SLOT(unselectWidgets()));
	connect(ui->footer, SIGNAL(focusIn()), this, SLOT(unselectWidgets()));
	
	new QShortcut(Qt::Key_PageUp, this, SLOT(scrollUp()));
	new QShortcut(Qt::Key_PageDown, this, SLOT(scrollDown()));
//	new QShortcut(Qt::ALT + Qt::Key_Delete, this, SLOT(removeBoth()));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_action_open_photorelation_triggered()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Otwórz fotorelację"), dirName, tr("Fotorelacje (*.phr)"));
	if (filePath.isEmpty())
		return;
	dirName = filePath.section(QDir::separator(), 0, -2);

	while (!ui->postLayout->isEmpty())
		delete ui->postLayout->takeAt(0)->widget();

	QFile file(filePath);
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);
	QString head, foot;
	qint32 count;
	QByteArray fileHeader;
	in >> fileHeader;
	if (fileHeader != phrFileHeader + qApp->applicationVersion().toAscii())
	{
		QMessageBox::critical(this, tr("Błąd"), fileHeader.startsWith(phrFileHeader) ? 
								  tr("Nie można otworzyć. Tę fotorelację zapisano inną wersją programu.") :
								  tr("To nie jest plik fotorelacji!"));
		return;
	}
	in >> head >> foot >> count >> *ui->commonMap;

	try
	{
		for (int i = 0; i < count; ++i)
			newImage("", &in);
	}
	catch (const Exception &e)
	{
		e.showMessage(this);
	}

	ui->header->setPlainText(head);
	ui->footer->setPlainText(foot);
	file.close();
	updateCommonMap();
}

void MainWindow::on_action_save_photorelation_triggered()
{
	if (ui->postLayout->isEmpty())
	{
		QMessageBox::warning(this, tr("Błąd"), tr("Fotorelacja nie zawiera żadnego zdjęcia."));
		return;
	}
	QString filePath = QFileDialog::getSaveFileName(this, tr("Zapisz fotorelację"), dirName, tr("Fotorelacje (*.phr)"));
	if (filePath.isEmpty())
		return;
	dirName = filePath.section(QDir::separator(), 0, -2);

	QFile file(filePath);
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	out << phrFileHeader + qApp->applicationVersion().toAscii() << ui->header->toPlainText() << ui->footer->toPlainText() << ui->postLayout->count() << *ui->commonMap;

	foreach (AbstractImage *image, imageList())
		image->serialize(out);
	file.close();
}

void MainWindow::on_action_add_photos_triggered()
{
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Wybierz zdjęcia"), dirName, tr("Zdjęcia (*.png *.gif *.jpg *.jpeg)"));
	if (files.isEmpty())
		return;
	dirName = files.first().section(QDir::separator(), 0, -2);

	try
	{
		foreach (QString file, files)
			newImage(file, 0);
	}
	catch (const Exception &e)
	{
		e.showMessage(this);
	}

	updateCommonMap();
}

void MainWindow::on_action_settings_triggered()
{
	settingsDialog.exec();
}

void MainWindow::on_action_import_gpx_triggered()
{
    if (gpxDialog.exec() == QDialog::Accepted)
		for (int i = 0; i < ui->postLayout->count(); ++i)
			static_cast<ImageWidget *>(ui->postLayout->itemAt(i)->widget())->setPosition(&gpxDialog);
	updateCommonMap();
}

void MainWindow::on_action_send_to_SSC_triggered()
{
	if (ui->postLayout->isEmpty())
	{
		QMessageBox::warning(this, tr("Błąd"), tr("Fotorelacja nie zawiera żadnego zdjęcia."));
		return;
	}

	QList<AbstractImage *> images = imageList();
	if (!ui->commonMap->isNull())
		images.prepend(ui->commonMap);

	images.first()->setHeader(ui->header->toPlainText());
	images.last()->setFooter(ui->footer->toPlainText());

	ReplyDialog reply(settings, images, this);
	
exec_reply:
	if (reply.exec() == QDialog::Rejected)
		if (QMessageBox::question(this, tr("Jeszcze raz"), tr("Czy spróbować dokończyć wysyłanie fotorelacji?"),
								  QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
			goto exec_reply;
	
	// update thread numbering 
	recentThreads.threadPosted(reply.threadId(), reply.threadTitle(), reply.latestPostedImageNumber());
	//		if (code == QDialog::Accepted)
	recentThreads.unselect();
}

void MainWindow::on_action_about_triggered()
{
	setWindowIcon(QIcon(":/res/help-about.png"));
	QMessageBox::about(this, tr("O programie"), tr(
						   "<h3>Fotorelacjonusz</h3><br/>Autor: Kamil Ostaszewski<br/>"
						   "<http://sourceforge.net/projects/fotorelacjonusz><br/><br/>"
						   "Aplikacja wykorzystuje:<br/>Qt (LGPL2)<br/>QuaZIP (LGPL2)<br/>Oxygen theme (LGPL)<br/><br/>%1")
					   .arg(QString(LICENSE).replace("\n", "<br/>")).replace(QRegExp("<(http://[^>]+)>"), "<a href='\\1'>\\1</a>"));
	setWindowIcon(QIcon());
}

void MainWindow::on_action_Qt_information_triggered()
{
	QMessageBox::aboutQt(this, tr("Wersja Qt"));
}

void MainWindow::moveImage(int number)
{
	if (!(0 <= number && number < ui->postLayout->count()))
		return;

	ui->scrollArea->ensureWidgetVisible(ui->postLayout->itemAt(number)->widget(), 0, 0);
	ui->postLayout->removeWidget(selectedImage);
	ui->postLayout->insertWidget(number, selectedImage);
	for (int i = qMin(number, selectedImage->number()), j = qMax(number, selectedImage->number()); i <= j; ++i)
		static_cast<ImageWidget *>(ui->postLayout->itemAt(i)->widget())->updateNumber(i);

	setTabOrder(number > 0 ? imageAt(number - 1)->lastWidget() : ui->header, selectedImage->firstWidget());
	setTabOrder(selectedImage->lastWidget(), number + 1 < ui->postLayout->count() ? imageAt(number + 1)->firstWidget() : ui->footer);
	updateCommonMap();
}

void MainWindow::on_action_move_top_triggered()
{
	moveImage(0);
}

void MainWindow::on_action_move_up_triggered()
{
	moveImage(selectedImage->number() - 1);
}

void MainWindow::on_action_move_down_triggered()
{
	moveImage(selectedImage->number() + 1);
}

void MainWindow::on_action_move_bottom_triggered()
{
	moveImage(ui->postLayout->count() - 1);
}

void MainWindow::on_action_rotate_left_triggered()
{
	selectedImage->rotate(true);
}

void MainWindow::on_action_rotate_right_triggered()
{
	selectedImage->rotate(false);
}

void MainWindow::on_action_remove_location_triggered()
{
	selectedImage->removePosition();
	updateCommonMap();
}

void MainWindow::on_action_remove_image_triggered()
{
	ui->imageToolBar->setEnabled(false);
	ui->postLayout->removeWidget(selectedImage);
	for (int i = selectedImage->number(); i < ui->postLayout->count(); ++i)
		imageAt(i)->updateNumber(i);
	delete selectedImage;
	selectedImage = 0;
	updateCommonMap();
}

void MainWindow::on_action_invert_colors_triggered()
{
	selectedArrow->invert();
}

void MainWindow::on_action_choose_color_triggered()
{
	selectedArrow->setColor(QColorDialog::getColor(Qt::black, this));
}

void MainWindow::on_action_remove_arrow_triggered()
{
	ui->arrowToolBar->setEnabled(false);
	delete selectedArrow;
	selectedArrow = 0;
}

void MainWindow::unselectWidgets()
{
	if (selectedImage)
		selectedImage->unselect();
	if (selectedArrow)
		selectedArrow->unselect();
}

void MainWindow::imageWidgetSelected(QWidget *widget)
{
	ImageWidget *image = qobject_cast<ImageWidget *>(widget);
//	qDebug() << "imageWidgetSelected" << widget << image;
	selectedImage = image;
	ui->imageToolBar->setEnabled(selectedImage);
	ui->colorManipulationBar->setWidget(selectedImage);
	
	if (selectedArrow && selectedImage)
		selectedArrow->unselect();
}

void MainWindow::arrowWidgetSelected(QWidget *widget)
{
	ArrowWidget *arrow = qobject_cast<ArrowWidget *>(widget);
//	qDebug() << "arrowWidgetSelected" << widget << arrow;
	selectedArrow = arrow;
	ui->arrowToolBar->setEnabled(selectedArrow);
	
	if (selectedImage && selectedArrow)
		selectedImage->unselect();
}

void MainWindow::updateCommonMap()
{
	if (settingsDialog.addCommonMap)
		gpsData.downloadMap();
	else
		ui->commonMap->setPixmap(QPixmap());
}

void MainWindow::commonMapReady(QImage map)
{
	ui->commonMap->setPixmap(QPixmap::fromImage(map));
}

void MainWindow::scrollUp()
{
	QScrollBar *bar = ui->scrollArea->verticalScrollBar();
	bar->setValue(bar->value() - bar->pageStep());
	
}

void MainWindow::scrollDown()
{
	QScrollBar *bar = ui->scrollArea->verticalScrollBar();
	bar->setValue(bar->value() + bar->pageStep());
}

void MainWindow::removeBoth()
{
	if (selectedImage)
		on_action_remove_image_triggered();
	else if (selectedArrow)
		on_action_remove_arrow_triggered();
}

void MainWindow::processEvents() const
{
//	QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents;
	QEventLoop::ProcessEventsFlags flags = QEventLoop::ExcludeUserInputEvents;
	ui->scrollArea->ensureVisible(0, 1000000);
	qApp->processEvents(flags);
	ui->scrollArea->ensureVisible(0, 1000000);
	qApp->processEvents(flags);
	ui->scrollArea->ensureVisible(0, 1000000);
}

ImageWidget *MainWindow::newImage(QString filePath, QDataStream *stream) throw(Exception)
{
	ImageWidget *widget = new ImageWidget(ui->postWidget, filePath, stream);
	ui->postLayout->addWidget(widget);
	setTabOrder(widget->lastWidget(), ui->footer);
	widget->setPosition(&gpxDialog);
	processEvents();
	return widget;
}

ImageWidget *MainWindow::imageAt(int i) const
{
	Q_ASSERT(0 <= i && i < ui->postLayout->count());
	return static_cast<ImageWidget *>(ui->postLayout->itemAt(i)->widget());
}

QList<AbstractImage *> MainWindow::imageList() const
{
	QList<AbstractImage *> images;
	for (int i = 0; i < ui->postLayout->count(); ++i)
		images << static_cast<ImageWidget *>(ui->postLayout->itemAt(i)->widget());
	return images;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	manager.save();
	QMainWindow::closeEvent(event);
}

bool MainWindow::isImageExtension(QString filePath)
{
	return (QStringList() << "jpg" << "jpeg" << "gif" << "png" << "bmp").contains(filePath.section(".", -1).toLower());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	foreach (QUrl url, event->mimeData()->urls())
		if (url.scheme() == "file" && isImageExtension(url.toLocalFile()))
			return event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
	QDateTime time = QDateTime::currentDateTime();
	try
	{
		foreach (QUrl url, event->mimeData()->urls())
			if (url.scheme() == "file" && isImageExtension(url.toLocalFile()))
				newImage(url.toLocalFile(), 0);
	}
	catch (const Exception &e)
	{
		e.showMessage(this);
	}

//	qDebug() << time.msecsTo(QDateTime::currentDateTime());
	
	event->acceptProposedAction();
	updateCommonMap();
}
