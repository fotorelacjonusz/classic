#ifndef COLORMANIPULATIONTOOLBAR_H
#define COLORMANIPULATIONTOOLBAR_H

#include <QToolBar>
#include <QSlider>

class ImageWidget;

class ColorManipulationToolBar : public QToolBar
{
	Q_OBJECT

public:
	explicit ColorManipulationToolBar(QWidget *parent = 0);
	~ColorManipulationToolBar();
	
public slots:
	void setWidget(ImageWidget *widget);
	
private slots:
	void on_colorManipulationBar_orientationChanged(Qt::Orientation orientation);

private:
	QSlider *contrastSlider, *brightnessSlider, *gammaSlider;
	
};

#endif // COLORMANIPULATIONTOOLBAR_H
