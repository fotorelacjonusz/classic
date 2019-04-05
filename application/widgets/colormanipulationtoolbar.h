#ifndef COLORMANIPULATIONTOOLBAR_H
#define COLORMANIPULATIONTOOLBAR_H

#include <QToolBar>

class ImageWidget;
class QSlider;

class ColorManipulationToolBar : public QToolBar
{
	Q_OBJECT

public:
	explicit ColorManipulationToolBar(QWidget *parent = nullptr);
	~ColorManipulationToolBar();

public slots:
	void setWidget(ImageWidget *widget);
	void on_action_undo_colors();

private slots:
	void on_colorManipulationBar_orientationChanged(Qt::Orientation orientation);

private:
	QSlider *contrastSlider, *brightnessSlider, *gammaSlider;

};

#endif // COLORMANIPULATIONTOOLBAR_H
