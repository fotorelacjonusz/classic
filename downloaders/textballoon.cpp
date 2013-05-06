#include "textballoon.h"

#include <QStringList>
#include <QPainter>
#include <QFile>
#include <QDebug>

TextBalloon::TextBalloon(QString fileName)
{
	// fileName format: anything.leftWidth-centerWidth-rightWidth.textLeft-textTop.fontSize.ext
	Q_ASSERT(QFile::exists(fileName));
	const QImage image(fileName);
	QStringList params = fileName.split('.');
	params.removeLast();
	font.setFamily("Dejavu Sans Mono");
	font.setBold(true);
	font.setPointSize(params.takeLast().toInt());
	Q_ASSERT(font.pointSize() > 0);
	const QStringList point = params.takeLast().split('-');
	topLeft.setX(point.first().toInt());
	topLeft.setY(point.last().toInt());
	Q_ASSERT(topLeft.x() > 0 && topLeft.y() > 0);
	const QStringList parts = params.takeLast().split('-');
	const int l = parts[0].toInt();
	const int c = parts[1].toInt();
	const int r = parts[2].toInt();
	Q_ASSERT(l > 0 && c > 0 && r > 0);
	const int h = image.height();
	left = image.copy(0, 0, l, h);
	center = image.copy(l, 0, c, h);
	right = image.copy(l + c, 0, r, h);
}

QImage TextBalloon::render(QString string)
{
	QImage text(QSize(100, left.height()), QImage::Format_ARGB32);
	text.fill(Qt::transparent);
	QRectF bounding;
	{
		QPainter painter(&text);
		painter.setFont(font);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.drawText(text.rect(), Qt::AlignLeft | Qt::AlignTop, string, &bounding);
	}
	
	QImage result(QSize(qMax(topLeft.x() + qRound(bounding.width()), left.width()) + right.width(), left.height()), QImage::Format_ARGB32);
	result.fill(Qt::transparent);
	{
		QPainter painter(&result);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		
		painter.drawImage(0, 0, left);
		painter.drawImage(left.width(), 0, center.scaled(result.width() - left.width() - right.width(), center.height()));
		painter.drawImage(result.width() - right.width(), 0, right);
		painter.drawImage(topLeft, text);
	}
	return result;
}

QImage TextBalloon::render(int number)
{
	return render(QString::number(number));
}
