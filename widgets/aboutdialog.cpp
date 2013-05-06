#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "exif/license.h"

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);
	setFixedSize(size());
	ui->textEdit->moveCursor(QTextCursor::End);
	ui->textEdit->insertPlainText(LICENSE);
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
