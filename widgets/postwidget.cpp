#include "postwidget.h"
#include "ui_postwidget.h"
#include "abstractimage.h"
#include "settings/settingsdialog.h"
#include <QToolBox>

PostWidget::PostWidget(QToolBox *parent) :
	QWidget(parent),
	ui(new Ui::PostWidget),
	isLast(false)
{
	ui->setupUi(this);
	
	openingTags = SETTINGS->extraTags.v().remove('\n');
	QRegExp tagExp("\\[([^=\\]]+)(=.+)?\\]");
	tagExp.setMinimal(true);
//	closingTags.replace(tagExp, "[/\\1]");
	for (int pos = 0; (pos = tagExp.indexIn(openingTags, pos)) != -1; pos += tagExp.matchedLength())
		closingTags.prepend("[/" + tagExp.cap(1) + "]");
	
	parent->addItem(this, tr("Post %1").arg(parent->count() + 1));
}

PostWidget::~PostWidget()
{
	delete ui;
}

void PostWidget::appendImage(AbstractImage *image)
{
	images.append(image);
	
	QString text;
	text += openingTags;
	foreach (AbstractImage *image, images)
		text += image->toBBCode() + "\n\n";
	if (SETTINGS->addTBC && !isLast)
		text += tr("Cdn ...");
	text += closingTags;
	ui->plainTextEdit->setPlainText(text);
}

int PostWidget::lastImageNumber() const
{
	return images.last()->number();
}

QString PostWidget::text() const
{
	return ui->plainTextEdit->toPlainText();
}

void PostWidget::setLast(bool last)
{
	isLast = last;
}

bool PostWidget::isFull() const
{
	return images.size() == SETTINGS->imagesPerPost;
}

