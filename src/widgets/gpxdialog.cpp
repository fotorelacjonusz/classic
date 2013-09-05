#include "gpxdialog.h"
#include "ui_gpxdialog.h"
#include "settings/settingsdialog.h"
#include "threadedvalidator.h"
#include <QFile>
#include <QDomDocument>
#include <QFileDialog>
#include <QTime>
#include <QMessageBox>
#include <QSourceLocation>

#define QDomFor(Var, Tag, Parent) (QDomElement Var = Parent.firstChildElement(Tag); !Var.isNull(); Var = Var.nextSiblingElement(Tag))

GpxDialog::GpxDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GpxDialog)
//	, validator(new ThreadedValidator(QUrl("http://www.topografix.com/GPX/1/1/gpx.xsd")))
{
	ui->setupUi(this);
	timer.setInterval(1000);
	connect(&timer, SIGNAL(timeout()), this, SLOT(updateTime()));
	connect(&ntpClient, SIGNAL(utcTimeFound(QDateTime)), ui->gpsTime, SLOT(setDateTime(QDateTime)));
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	
	qRegisterMetaType<QtMsgType>("QtMsgType");
	qRegisterMetaType<QSourceLocation>("QSourceLocation");
	
//	connect(validator, SIGNAL(validated(bool)), this, SLOT(validated(bool)));
//	connect(validator, SIGNAL(message(QtMsgType,QString,QUrl,QSourceLocation)), this, SLOT(message(QtMsgType,QString,QUrl,QSourceLocation)));
}

GpxDialog::~GpxDialog()
{
	delete ui;
//	delete validator;
}

void GpxDialog::setVisible(bool visible)
{
	QDialog::setVisible(visible);
	if (visible && !timer.isActive())
	{
		ui->cameraTime->setDateTime(QDateTime::currentDateTime());
//		ui->gpsTime->setDateTime(QDateTime::currentDateTime());
		ntpClient.updateTime();
		timer.start();
	}
}

void GpxDialog::accept()
{
	if (ntpClient.utcTime().isNull())
	{
		QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się ustalić czasu UTC poprzez protokół NTP."));
		done(Rejected);
		return;
	}
	
	qint64 diff = ntpClient.utcTime().msecsTo(ui->cameraTime->dateTime().toUTC()); // camera - gps
	
	track.clear();
	
	QFile gpxFile(ui->gpxFile->text());
	gpxFile.open(QIODevice::ReadOnly);
	QDomDocument doc;
	doc.setContent(&gpxFile);
	
	for QDomFor(trk, "trk", doc.documentElement())
		for QDomFor(trkseg, "trkseg", trk)
		{
			QMap<QDateTime, QPointF> segment;
			for QDomFor(trkpt, "trkpt", trkseg)
			{
				QPointF pos(trkpt.attribute("lon").toDouble(), trkpt.attribute("lat").toDouble());
				QDateTime dt = QDateTime::fromString(trkpt.firstChildElement("time").text(), Qt::ISODate);
				if (pos.isNull() || !dt.isValid())
					continue;
				segment.insert(dt.toLocalTime().addMSecs(diff), pos);
			}
			track.append(segment);
		}
//	qDebug() << track;
	
	done(Accepted);
}

void GpxDialog::reject()
{
	done(Rejected);
}

QPointF GpxDialog::position(QDateTime dateTime) const
{
	dateTime = dateTime.toLocalTime(); 
	if (!dateTime.isValid())
		return QPointF();
	foreach (Segment segment, track)
	{
		Segment::Iterator i = segment.lowerBound(dateTime);
		if (i != segment.begin() && i != segment.end())
		{
			// interpolate
			const qreal s = qreal((i - 1).key().msecsTo(dateTime)) / (i - 1).key().msecsTo(i.key());
			return (i - 1).value() * (1.0 - s) + i.value() * s;
		}
	}
	return QPointF();
}

bool GpxDialog::updateFiles() const
{
	return ui->updateFiles->isChecked();
}

void GpxDialog::message(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
{
	QString text = tr("Błąd %1 w %2<br/>Wiersz %3, kolumna %4:%5").arg(identifier.fragment()).arg(sourceLocation.uri().toString())
				   .arg(sourceLocation.line()).arg(sourceLocation.column()).arg(description);
	if (type == QtCriticalMsg || type == QtFatalMsg)
		QMessageBox::critical(this, tr("Błąd"), text);
	else
		qDebug() << text;
}

void GpxDialog::validated(bool valid)
{
	ui->loadButton->setEnabled(true);
	if (valid)
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void GpxDialog::updateTime()
{
	ui->cameraTime->setDateTime(ui->cameraTime->dateTime().addSecs(1));
	ui->gpsTime->setDateTime(ui->gpsTime->dateTime().addSecs(1));
}

void GpxDialog::on_loadButton_clicked()
{
	QString dirPath = SETTINGS->settings().value("gpx_dir").toString();
	QString filePath = QFileDialog::getOpenFileName(this, tr("Wybierz plik"), dirPath, tr("Track (*.gpx)"));
	if (filePath.isEmpty())
		return;
	
	SETTINGS->settings().setValue("gpx_dir", filePath.section('/', 0, -2));
		
	ui->gpxFile->setText(filePath);
	ui->loadButton->setEnabled(false);
		
//	validator->validate(QUrl::fromLocalFile(filePath));
	
	ui->loadButton->setEnabled(true);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
