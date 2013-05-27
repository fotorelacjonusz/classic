#include "gpxdialog.h"
#include "ui_gpxdialog.h"
#include "downloaders/throttlednetworkmanager.h"
#include <QFile>
#include <QDomDocument>
#include <QFileDialog>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QAbstractMessageHandler>
#include <QMessageBox>
#include <QDebug>

#define QDomFor(Var, Tag, Parent) (QDomElement Var = Parent.firstChildElement(Tag); !Var.isNull(); Var = Var.nextSiblingElement(Tag))

class MessageHandler : public QAbstractMessageHandler
{
public:
	MessageHandler(QWidget *parent):
		QAbstractMessageHandler(parent),
		parent(parent)
	{
	}
	
	virtual void handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
	{
		QString text = tr("Błąd %1 w %2<br/>Wiersz %3, kolumna %4:%5").arg(identifier.fragment()).arg(sourceLocation.uri().toString())
					   .arg(sourceLocation.line()).arg(sourceLocation.column()).arg(description);
		if (type == QtCriticalMsg || type == QtFatalMsg)
			QMessageBox::critical(parent, tr("Błąd"), text);
		else
			qDebug() << text;
	}
	
private:
	QWidget * const parent;
};

GpxDialog::GpxDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GpxDialog),
	handler(new MessageHandler(this))
{
	ui->setupUi(this);
	timer.setInterval(1000);
	connect(&timer, SIGNAL(timeout()), this, SLOT(updateTime()));
}

GpxDialog::~GpxDialog()
{
	delete ui;
}

void GpxDialog::setVisible(bool visible)
{
	QDialog::setVisible(visible);
	if (visible)
	{
		ui->cameraTime->setDateTime(QDateTime::currentDateTime());
		ui->gpsTime->setDateTime(QDateTime::currentDateTime());
		timer.start();		
	}
}

void GpxDialog::accept()
{
	qint64 diff = ui->gpsTime->dateTime().msecsTo(ui->cameraTime->dateTime()); // camera - gps
	
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
	
	QDialog::done(Accepted);
}

void GpxDialog::reject()
{
	QDialog::done(Rejected);
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

void GpxDialog::updateTime()
{
	ui->cameraTime->setDateTime(ui->cameraTime->dateTime().addSecs(1));
	ui->gpsTime->setDateTime(ui->gpsTime->dateTime().addSecs(1));
}

void GpxDialog::on_loadButton_clicked()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Wybierz plik"), "../", tr("Track (*.gpx)"));
	static ThrottledNetworkManager manager;
	QXmlSchema schema;
	schema.setNetworkAccessManager(manager.manager());
	schema.load(QUrl("http://www.topografix.com/GPX/1/1/gpx.xsd"));
	Q_ASSERT(schema.isValid());
	QXmlSchemaValidator validator(schema);
	validator.setMessageHandler(handler);
	if (!validator.validate(QUrl::fromLocalFile(filePath)))
		return;

	ui->gpxFile->setText(filePath);
	ui->buttonBox->addButton(QDialogButtonBox::Ok);
}
