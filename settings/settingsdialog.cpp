#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "uploaders/abstractuploader.h"

#include <QDebug>
#include <QFileDialog>
#include <QBuffer>
#include <QColorDialog>
#include <QMetaEnum>
#include <QFontDialog>
#include <QTimer>
#include <QMessageBox>
#include <QIntValidator>

#define ENUM_STR(Enum, Var) QString(metaObject()->enumerator(metaObject()->indexOfEnumerator(#Enum)).valueToKey(Var))
#define M(x) #x

SettingsDialog *SettingsDialog::objectInstance = 0;

SettingsDialog::SettingsDialog(QWidget *parent, QSettings &settings) :
	QDialog(parent),
	SettingsManager(settings),
	ui(new Ui::SettingsDialog),
	m_uploader(0),
	m_settings(settings),
	osmDialog(settings, this)
{
	objectInstance = this;

	ui->setupUi(this);
	ui->jpgQuality->setFormat("%1 %");
	ui->imageMapOpacity->setFormat("%1 %");
	ui->homePageGroup->setId(ui->homeMainRadio, 0);
	ui->lengthComboBox->setValidator(new QIntValidator(300, 1920, this));
	tosFormat = ui->tosLabel->text();
	connect(ui->openFolderButton, SIGNAL(clicked()), ui->overlayList, SLOT(openFolder()));
	osmDialog.init(ui);
	cornerSpecyficImageMapOptions << ui->imageMapCircle << ui->imageMapCircleLabel << ui->imageMapOpacity << ui->imageMapOpacityLabel
								  << ui->imageMapMargin << ui->imageMapMarginLabel;

	// earlier "logo" was wrongly called "watermark"
	if (settings.childGroups().contains("watermark"))
		foreach (QString key, settings.allKeys())
			if (key.startsWith("watermark"))
				settings.setValue(key.repeated(1).replace("watermark/", "logo/"), settings.value(key));
	
	// fix alignment between two form layouts in two group boxes
	int maxWidth = 0;
	foreach (QObject *object, ui->commonMapBox->children() + ui->imageMapBox->children())
	{
		QLabel *label = qobject_cast<QLabel *>(object);
		if (label)
			maxWidth = qMax(maxWidth, label->sizeHint().width());
	}
	ui->commonMapTypeLabel->setMinimumWidth(maxWidth);	
	ui->commonMapTypeLabel->setAlignment(qobject_cast<QFormLayout *>(ui->commonMapBox->layout())->labelAlignment() | 
										 (ui->imageMapTypeLabel->alignment() & 0x00f0));
	

	uploader			.init(makeInput("upload_method",              ui->uploadMethodComboBox,  0),   this, &SettingsDialog::uploaderFunc);
							  makeInput("home_page/tag",              ui->homeTag);
							  makeInput("home_page/forum_id",         ui->homeForumId);
	homeUrl             .init(makeInput("home_page/method",           ui->homePageGroup,         0),   this, &SettingsDialog::homeUrlFunc);
	captionsUnder       .init(makeInput("captions_under",             ui->captionsPositionGroup, -1),  this, &SettingsDialog::captionsUnderFunc);
	extraSpace          .init(makeInput("caption_extra_space",        ui->extraSpace,			 false));
	numberImages        .init(makeInput("number_images",              ui->numberImages,	         true));
	startingNumber      .init(makeInput("starting_number",            ui->startingNumber,        1),   this, &SettingsDialog::startingNumberFunc);
	addImageBorder      .init(makeInput("image_add_border",           ui->addImageBorder,		 false));
	addTBC              .init(makeInput("add_tbc",                    ui->addTBC,                true));
	imagesPerPost       .init(makeInput("images_per_post",            ui->imagesPerPost,         5));
	postSpace			.init(makeInput("post_space",                 ui->postSpace,             60));
	extraTags           .init(makeInput("post_extra_tags",            ui->extraTags));

	setImageWidth       .init(makeInput("image_size/set_width",       ui->imageScaleMethodGroup, -1),  this, &SettingsDialog::setImageWidthFunc);
	imageLength         .init(makeInput("image_size/lenght",          ui->lengthComboBox,        2),   this, &SettingsDialog::imageLengthFunc);
	retainOriginalSize  .init(makeInput("image_size/retain_size",     ui->retainOriginalSize,    false));
	jpgQuality          .init(makeInput("image_size/jpg_quality",     ui->jpgQuality,            70));
	dontScalePanoramas  .init(makeInput("image/dont_scale_panoramas", ui->dontScalePanoramas,    true));

	addLogo             .init(makeInput("logo/add",                   ui->logoGroupBox,          false));
	logoPixmap			.init(makeInput("logo/data",				  &logo));
	logoCorner			.init(makeInput("logo/position",              ui->logoPosition,          0),   this, &SettingsDialog::logoCornerFunc);
	logoMargin			.init(makeInput("logo/margin",                ui->logoMargin,            0));
	logoInvert			.init(makeInput("logo/invert",				  ui->logoInvert,            false));

	addCommonMap        .init(makeInput("common_map/add",	          ui->commonMapBox,          true));
	addImageMap         .init(makeInput("image_map/add",              ui->imageMapBox,           true));
	commonMapType       .init(makeInput("common_map/type",            ui->commonMapType,         2),   this, &SettingsDialog::commonMapTypeFunc);
	imageMapType        .init(makeInput("image_map/type",             ui->imageMapType,          2),   this, &SettingsDialog::imageMapTypeFunc);
	imageMapOpacity     .init(makeInput("image_map/opacity",          ui->imageMapOpacity,       100), this, &SettingsDialog::imageMapOpacityFunc);
	imageMapCorner      .init(makeInput("image_map/position",         ui->imageMapPosition,      2),   this, &SettingsDialog::imageMapCornerFunc);
	imageMapColor       .init(makeInput("image_map/color",            &m_imageMapColor,          Qt::red));
	imageMapZoom        .init(makeInput("image_map/zoom",             ui->imageMapZoom,          10));
	imageMapCircle      .init(makeInput("image_map/circle",           ui->imageMapCircle,        false));
	imageMapMargin      .init(makeInput("image_map/margin",           ui->imageMapMargin,        10));
	imageMapSize        .init(makeInput("image_map/size",             ui->imageMapSize,          150));

	useOverlays         .init(makeInput("overlays/use",	              ui->useOverlays,           true));
	useOverlayCommonMap .init(makeInput("overlays/use_on_common",	  ui->useOverlayCommonMap,   true));
	
	useProxy            .init(makeInput("proxy/use",                  ui->useProxy,              false));
	proxyHost           .init(makeInput("proxy/host",                 ui->proxyHost));
	proxyPort			.init(makeInput("proxy/port",                 ui->proxyPort,			 8080));
	proxyUser           .init(makeInput("proxy/user",                 ui->proxyUser));
	proxyPass           .init(makeInput("proxy/pass",                 ui->proxyPass));
	
	connectMany(this, SLOT(proxyOptionsChanged()), &useProxy, &proxyHost, &proxyPort, &proxyUser, &proxyPass);

	for (int i = 0; i < factory.uploaders.size(); ++i)
		ui->uploadMethodComboBox->addItem(factory.uploaders[i]->name);

	reject();
}

SettingsDialog::~SettingsDialog()
{
	objectInstance = 0;
	delete m_uploader;
	delete ui;
}

void SettingsDialog::copyDescriptions(QWidget *parent)
{
	foreach (QObject *obj, parent->children())
	{
		if (obj->isWidgetType())
		{
			QWidget *widget = (QWidget *)obj;
			QString description = widget->whatsThis() + widget->statusTip() + widget->toolTip();
			widget->setWhatsThis(description);
			widget->setToolTip(description);
			widget->setStatusTip(description);
			if (widget->inherits("QToolBar"))
				widget->setWindowTitle(description);
			if (widget->inherits("QAction"))
			{
				QAction *action = (QAction *)widget;
				action->setText(description);
				action->setIconText(description);
			}
			copyDescriptions(widget);
		}
	}
}

void SettingsDialog::setSelectedThread(QString threadId, int number)
{
	selectedThreadId = threadId;
	selectedThreadImageNumber = number;
	startingNumber.changedRemotely();
//	emit numberOptionsChanged();
}

bool SettingsDialog::isSelectedThread()                               const { return !selectedThreadId.isEmpty(); }
bool SettingsDialog::makeMap(GeoMap *map)                                   { return ui->overlayList->makeMap(map); }
SettingsDialog *SettingsDialog::object()                                    { return objectInstance; }
QSettings &SettingsDialog::settings()                                       { return m_settings; }
QString SettingsDialog::mapTypeToString(SettingsDialog::MapType type) const { return ENUM_STR(MapType, type).toLower(); }
OSMLayer SettingsDialog::currentOsmLayer(bool common)                 const { return osmDialog.currentLayer(common); }
AbstractUploader *SettingsDialog::uploaderFunc()		              const { return m_uploader; }
bool SettingsDialog::captionsUnderFunc()                              const { return ui->captionsUnder->isChecked(); }
int SettingsDialog::startingNumberFunc()                              const { return selectedThreadId.isEmpty() ? ui->startingNumber->value() : selectedThreadImageNumber; }
bool SettingsDialog::setImageWidthFunc()                              const { return ui->widthRadioButton->isChecked(); }
int SettingsDialog::imageLengthFunc()                                 const { return ui->lengthComboBox->currentText().toInt(); }
SettingsDialog::Corner SettingsDialog::logoCornerFunc()               const { return (Corner)ui->logoPosition->currentIndex(); }
SettingsDialog::MapType SettingsDialog::commonMapTypeFunc()           const { return (MapType)ui->commonMapType->currentIndex(); }
SettingsDialog::MapType SettingsDialog::imageMapTypeFunc()            const { return (MapType)ui->imageMapType->currentIndex(); }
qreal SettingsDialog::imageMapOpacityFunc()                           const { return ui->imageMapOpacity->value() / 100.0; }
SettingsDialog::Corner SettingsDialog::imageMapCornerFunc()           const { return (Corner)ui->imageMapPosition->currentIndex(); }

QUrl SettingsDialog::homeUrlFunc() const
{
	QUrl url(SSC_HOST);
	if (!selectedThreadId.isEmpty())
	{
		url.setPath("/showthread.php");
		url.addQueryItem("t", selectedThreadId);
	}
	else if (ui->homeMainRadio->isChecked())
		return url;
	else if (ui->homeUserCPRadio->isChecked())
		url.setPath("/usercp.php");
	else if (ui->homeForumRadio->isChecked())
	{
		url.setPath("/forumdisplay.php");
		url.addQueryItem("f", ui->homeForumId->cleanText());
	}
	else if (ui->homeTagRadio->isChecked())
	{
		QString homeTag;
		foreach (uint i, ui->homeTag->text().toUcs4())
			homeTag.append(i <= 255 ? QString(QChar(i)) : QString("&#%1;").arg(i));

		url.setPath("/tags.php");
		url.addEncodedQueryItem("tag", homeTag.toLatin1().toPercentEncoding());
	}
	return url;
}

void SettingsDialog::fixLengthComboBox()
{
	int min = qobject_cast<const QIntValidator *>(ui->lengthComboBox->validator())->bottom();
	if (ui->lengthComboBox->currentText().toInt() < min)
		ui->lengthComboBox->setCurrentIndex(0);
}

void SettingsDialog::accept()
{
	fixLengthComboBox();
	
	if (ui->imageMapType->currentText().isEmpty())
		ui->imageMapType->setCurrentIndex(0);
	if (ui->commonMapType->currentText().isEmpty())
		ui->commonMapType->setCurrentIndex(0);
	save();
	m_uploader->save();
	QDialog::accept();
}

void SettingsDialog::reject()
{
	load();
	
	fixLengthComboBox();
	
	ui->logoLabel->setPixmap(logo);
	ui->imageMapColor->setStyleSheet("background-color: " + m_imageMapColor.name());
	
	if (ui->imageMapType->currentText().isEmpty())
	{
		ui->imageMapType->setCurrentIndex(0);
		save(imageMapType.getInput());
	}
	if (ui->commonMapType->currentText().isEmpty())
	{
		ui->commonMapType->setCurrentIndex(0);
		save(commonMapType.getInput());
	}
	
	QDialog::reject();
}

void SettingsDialog::on_logoLoad_clicked()
{
	QString logoDir = m_settings.value("logo/dir").toString();
	QString logoFile = QFileDialog::getOpenFileName(this, tr("Wczytaj logo"), logoDir, tr("Zdjęcia (*.png *.gif *.jpg *.jpeg)"));
	if (!logoFile.isEmpty())
	{
		m_settings.setValue("logo/dir", logoFile.section("/", 0, -2));
		logo.load(logoFile);
		ui->logoLabel->setPixmap(logo);
	}
}

void SettingsDialog::on_uploadMethodComboBox_currentIndexChanged(int index)
{
	if (m_uploader)
	{
		ui->uploadGroupBox->layout()->removeWidget(m_uploader);
		m_uploader->setVisible(false);
	}

	m_uploader = factory.uploaders[index]->make(ui->uploadGroupBox, m_settings);
	m_uploader->setVisible(true);
	ui->uploadGroupBox->layout()->addWidget(m_uploader);

	QWidget *last = m_uploader->lastInputWidget();
	if (last)
	{
		QList<QAbstractButton *> buttons = ui->buttonBox->buttons();
		setTabOrder(last, buttons.first());
		for (int i = 0; i < buttons.size() - 1; ++i)
			setTabOrder(buttons[i], buttons[i + 1]);
	}
	QString tosUrl = m_uploader->tosUrl();
	ui->tosLabel->setVisible(!tosUrl.isEmpty());
	ui->tosLabel->setText(tosFormat.arg(ui->uploadMethodComboBox->currentText().section(" ", 0, 0)).arg(tosUrl));
	copyDescriptions(m_uploader);
	QTimer::singleShot(0, m_uploader, SLOT(load())); // loading might require some blocking http requests
//	m_uploader->load();
}

void SettingsDialog::on_imageMapColor_clicked()
{
	QColor color = QColorDialog::getColor(m_imageMapColor, this);
	if (!color.isValid())
		return;

	m_imageMapColor = color;
	ui->imageMapColor->setStyleSheet("background-color: " + m_imageMapColor.name());
}

void SettingsDialog::on_imageMapPosition_currentIndexChanged(int index)
{
	bool expand = index >= Expand;
	foreach (QWidget *widget, cornerSpecyficImageMapOptions)
		widget->setEnabled(!expand);
}

void SettingsDialog::proxyOptionsChanged()
{
	proxy.setType(useProxy ? QNetworkProxy::Socks5Proxy : QNetworkProxy::NoProxy);
	proxy.setHostName(proxyHost);
	proxy.setPort(proxyPort);
	if (!proxyUser.v().isEmpty() && !proxyPass.v().isEmpty())
	{
		proxy.setUser(proxyUser);
		proxy.setPassword(proxyPass);
	}
	QNetworkProxy::setApplicationProxy(proxy);
//	qDebug() << "proxy set";
}
