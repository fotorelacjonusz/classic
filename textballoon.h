#ifndef TEXTBALLOON_H
#define TEXTBALLOON_H

#include <QString>
#include <QImage>
#include <QFont>

class TextBalloon
{
public:
	TextBalloon(QString fileName);
	QImage render(QString string);
	QImage render(int number);
	
private:
	QImage left, center, right;
	QPoint topLeft;
	QFont font;
};

#endif // TEXTBALLOON_H
