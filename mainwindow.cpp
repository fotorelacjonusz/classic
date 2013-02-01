#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imagewidget.h"
#include "abstractimage.h"
#include "simpleimage.h"
#include "arrowwidget.h"
#include "aboutdialog.h"
#include "replydialog.h"
#include "gpsdata.h"

#include <QUrl>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>

QByteArray MainWindow::phrFileHeader("PHR PHotoRelation file. Program info: http://www.skyscrapercity.com/showthread.php?t=1539539 Author: Kamil Ostaszewski v2.2");

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	manager(settings),
	settingsDialog(this, settings),
	currentImage(0),
	currentArrow(0)
{
	ui->setupUi(this);
	ui->imageToolBar->setEnabled(false);
	ui->arrowToolBar->setEnabled(false);
	ui->colorManipulationBar->setEnabled(false);
	settingsDialog.copyDescriptions(this);

	manager.makeInput("geometry", (QWidget *)this);
	manager.makeInput("window_state", (QMainWindow *)this);
	manager.makeInput("dir_name", &dirName);
	manager.load();

	connect(&settingsDialog, SIGNAL(commonMapOptionsChanged()), this, SLOT(updateCommonMap()));
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
	if (fileHeader != phrFileHeader)
	{
		QMessageBox::critical(this, tr("Błąd"), tr("To nie jest plik fotorelacji!"));
		return;
	}
	in >> head >> foot >> count;

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
	out << phrFileHeader << ui->header->toPlainText() << ui->footer->toPlainText() << ui->postLayout->count();

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

void MainWindow::on_action_send_to_SSC_triggered()
{
	if (ui->postLayout->isEmpty())
	{
		QMessageBox::warning(this, tr("Błąd"), tr("Fotorelacja nie zawiera żadnego zdjęcia."));
		return;
	}

	QList<AbstractImage *> images = imageList();
	SimpleImage *mapImage = 0;
	if (ui->commonMap->pixmap() && !ui->commonMap->pixmap()->isNull())
	{
		mapImage = new SimpleImage(ui->commonMap->pixmap(), tr("Mapa dla wszystkich zdjęć."));
		images.prepend(mapImage);
	}

	images.first()->prepend(ui->header->toPlainText());
	images.last()->append(ui->footer->toPlainText());

	ReplyDialog reply(images, this);
	reply.exec();

	if (mapImage)
		delete mapImage;
}

void MainWindow::on_action_about_triggered()
{
	AboutDialog().exec();
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
	ui->postLayout->removeWidget(currentImage);
	ui->postLayout->insertWidget(number, currentImage);
	for (int i = qMin(number, currentImage->getNumber()), j = qMax(number, currentImage->getNumber()); i <= j; ++i)
		static_cast<ImageWidget *>(ui->postLayout->itemAt(i)->widget())->updateNumber(i);

	setTabOrder(number > 0 ? imageAt(number - 1)->getCaptionEdit() : ui->header, currentImage->getCaptionEdit());
	setTabOrder(currentImage->getCaptionEdit(), number + 1 < ui->postLayout->count() ? imageAt(number + 1)->getCaptionEdit() : ui->footer);
}

void MainWindow::on_action_move_top_triggered()
{
	moveImage(0);
}

void MainWindow::on_action_move_up_triggered()
{
	moveImage(currentImage->getNumber() - 1);
}

void MainWindow::on_action_move_down_triggered()
{
	moveImage(currentImage->getNumber() + 1);
}

void MainWindow::on_action_move_bottom_triggered()
{
	moveImage(ui->postLayout->count() - 1);
}

void MainWindow::on_action_rotate_left_triggered()
{
	currentImage->rotate(true);
}

void MainWindow::on_action_rotate_right_triggered()
{
	currentImage->rotate(false);
}

void MainWindow::on_action_remove_image_triggered()
{
	ui->imageToolBar->setEnabled(false);
	ui->postLayout->removeWidget(currentImage);
	for (int i = currentImage->getNumber(); i < ui->postLayout->count(); ++i)
		imageAt(i)->updateNumber(i);
	delete currentImage;
	currentImage = 0;
	updateCommonMap();
}

void MainWindow::on_action_invert_colors_triggered()
{
	currentArrow->invert();
}

void MainWindow::on_action_choose_color_triggered()
{
	currentArrow->setColor(QColorDialog::getColor(Qt::black, this));
}

void MainWindow::on_action_remove_arrow_triggered()
{
	ui->arrowToolBar->setEnabled(false);
	delete currentArrow;
	currentArrow = 0;
}

void MainWindow::imageWidgetSelected(ImageWidget *widget)
{
	if (currentImage)
	{
		currentImage->unselected();
		ui->colorManipulationBar->imageUnselected();
	}
	currentImage = currentImage != widget ? widget : 0;
	ui->imageToolBar->setEnabled(currentImage);
	ui->colorManipulationBar->setEnabled(currentImage);
	if (widget)
	{
		arrowWidgetSelected(0);
		ui->colorManipulationBar->imageSelected(widget);
	}
}

void MainWindow::arrowWidgetSelected(ArrowWidget *widget)
{
	if (currentArrow)
		currentArrow->unselected();
	currentArrow = currentArrow != widget ? widget : 0;
	ui->arrowToolBar->setEnabled(currentArrow);
	if (widget)
		imageWidgetSelected(0);
}

void MainWindow::updateCommonMap()
{
	if (!settingsDialog.addCommonMap)
	{
		ui->commonMap->setPixmap(QPixmap());
		return;
	}

	GpsData gpsData;
	QPixmap map = gpsData.downloadCommonMap();
	if (map.isNull())
		return;
	ui->commonMap->setPixmap(map);
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
	connect(widget, SIGNAL(selected(ImageWidget*)), this, SLOT(imageWidgetSelected(ImageWidget*)));
	connect(widget, SIGNAL(selected(ArrowWidget*)), this, SLOT(arrowWidgetSelected(ArrowWidget*)));
	setTabOrder(widget->getCaptionEdit(), ui->footer);
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
		{
			event->acceptProposedAction();
			return;
		}
}

void MainWindow::dropEvent(QDropEvent *event)
{
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

	event->acceptProposedAction();
	updateCommonMap();
}







