#include "colormanipulationtoolbar.h"
#include "imagewidget.h"
#include "imagemanipulation.h"
#include <QLayout>

ColorManipulationToolBar::ColorManipulationToolBar(QWidget *parent) :
	QToolBar(parent)
{
	setStyleSheet("QSlider::handle			  { background-color: #ff3311; border: 1px solid #5c5c5c; }"
				  "QSlider::handle:disabled   { background-color: #777777; }"
				  "QSlider::handle:vertical   { width: 20px; height: 4px; }"
				  "QSlider::handle:horizontal { width: 4px; height: 20px; }");

	contrastSlider = new QSlider(Qt::Vertical, this);
	addWidget(contrastSlider);
	layout()->setAlignment(contrastSlider, Qt::AlignCenter);
	contrastSlider->setToolTip(tr("Kontrast"));
	contrastSlider->setMinimum(CONTRAST_MIN);
	contrastSlider->setMaximum(CONTRAST_MAX);
	contrastSlider->setValue(CONTRAST_DEFAULT);
	contrastSlider->setStyleSheet("QSlider::groove:vertical   { background: qconicalgradient(cx:0.5, cy:0.5, angle:90, stop:0 white, stop:1 black); }"
								  "QSlider::groove:horizontal { background: qconicalgradient(cx:0.5, cy:0.5, angle:0,  stop:0 white, stop:1 black); }");

	brightnessSlider = new QSlider(Qt::Vertical, this);
	addWidget(brightnessSlider);
	layout()->setAlignment(brightnessSlider, Qt::AlignCenter);
	brightnessSlider->setToolTip(tr("Jasność"));
	brightnessSlider->setMinimum(BRIGHTNESS_MIN);
	brightnessSlider->setMaximum(BRIGHTNESS_MAX);
	brightnessSlider->setValue(BRIGHTNESS_DEFAULT);
	brightnessSlider->setStyleSheet("QSlider::groove:vertical   { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 white, stop:1 black); }"
									"QSlider::groove:horizontal { background: qlineargradient(x1:1, y1:0, x2:0, y2:0, stop:0 white, stop:1 black); }");

	gammaSlider = new QSlider(Qt::Vertical, this);
	addWidget(gammaSlider);
	layout()->setAlignment(gammaSlider, Qt::AlignCenter);
	gammaSlider->setToolTip(tr("Gamma"));
	gammaSlider->setMinimum(GAMMA_MIN);
	gammaSlider->setMaximum(GAMMA_MAX);
	gammaSlider->setValue(GAMMA_DEFAULT);
	gammaSlider->setStyleSheet("QSlider::groove:vertical   { background: qconicalgradient(cx:0.5, cy:0.5, angle:270, stop:0 white, stop:0.5 white, stop:1 black); }"
							   "QSlider::groove:horizontal { background: qconicalgradient(cx:0.5, cy:0.5, angle:270, stop:0 white, stop:0.5 white, stop:1 black); }");

	connect(this, SIGNAL(orientationChanged(Qt::Orientation)), this, SLOT(on_colorManipulationBar_orientationChanged(Qt::Orientation)));
	on_colorManipulationBar_orientationChanged(orientation());
}

ColorManipulationToolBar::~ColorManipulationToolBar()
{
	delete contrastSlider;
	delete brightnessSlider;
	delete gammaSlider;
}

void ColorManipulationToolBar::imageSelected(ImageWidget *widget)
{
	brightnessSlider->setValue(widget->getBrightness());
	contrastSlider->setValue(widget->getContrast());
	gammaSlider->setValue(widget->getGamma());
	connect(brightnessSlider, SIGNAL(valueChanged(int)), widget, SLOT(setBrightness(int)));
	connect(contrastSlider, SIGNAL(valueChanged(int)), widget, SLOT(setContrast(int)));
	connect(gammaSlider, SIGNAL(valueChanged(int)), widget, SLOT(setGamma(int)));
}

void ColorManipulationToolBar::imageUnselected()
{
	disconnect(brightnessSlider, SIGNAL(valueChanged(int)), 0, 0);
	disconnect(contrastSlider, SIGNAL(valueChanged(int)), 0, 0);
	disconnect(gammaSlider, SIGNAL(valueChanged(int)), 0, 0);
}

void ColorManipulationToolBar::on_colorManipulationBar_orientationChanged(Qt::Orientation orientation)
{
	contrastSlider->setOrientation(orientation);
	brightnessSlider->setOrientation(orientation);
	gammaSlider->setOrientation(orientation);

	int w = (orientation == Qt::Vertical) ? 20 : 150;
	int h = (orientation == Qt::Vertical) ? 150 : 20;

	contrastSlider->setFixedSize(w, h);
	brightnessSlider->setFixedSize(w, h);
	gammaSlider->setFixedSize(w, h);
}

