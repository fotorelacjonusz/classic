#include "postwidget.h"
#include "ui_postwidget.h"
#include <QToolBox>

PostWidget::PostWidget(QToolBox *parent) :
	QWidget(parent),
	ui(new Ui::PostWidget),
	m_ready(false)
{
	ui->setupUi(this);
}

PostWidget::~PostWidget()
{
	delete ui;
}

void PostWidget::append(QString text, bool ready)
{
	ui->plainTextEdit->setPlainText(ui->plainTextEdit->toPlainText() + text);
	if (ready)
		m_ready = true;
	emit appended(100);
}

QString PostWidget::text() const
{
	return ui->plainTextEdit->toPlainText();
}

bool PostWidget::isReady() const
{
	return m_ready;
}

