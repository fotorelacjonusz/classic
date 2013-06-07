#include "questionbox.h"
#include "settings/settingsdialog.h"

#include <QCheckBox>
#include <QDebug>
#include <QLayout>
//#include <QLayoutItem>
#include <QVBoxLayout>
#include <QTimer>

QMap<QString, bool> QuestionBox::answers;

QuestionBox::QuestionBox(const QString &title, const QString &question, const QString &settingsKey, QWidget *parent) :
	QMessageBox(Question, title, question, Yes | No, parent),
	settingsKey(settingsKey)
{
	QGridLayout *gridLayout = qobject_cast<QGridLayout *>(layout());
	if (!gridLayout)
	{
		qDebug() << "QMessageBox.layout() is not of type QGridLayout!";
		return;
	}
		
	QWidget *widget = new QWidget(this);
	checkBoxForever = new QCheckBox(tr("Zapamiętaj na zawsze"), widget);
	checkBoxSession = new QCheckBox(tr("Zapamiętaj w tej sesji"), widget);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(checkBoxForever);
	layout->addWidget(checkBoxSession);
	layout->setContentsMargins(5, 5, 5, 5);
	widget->setLayout(layout);
	
	gridLayout->addWidget(widget, 1, 1, 1, 1); // only empty cell i grid, under the text
	
	connect(checkBoxForever, SIGNAL(toggled(bool)), checkBoxSession, SLOT(setDisabled(bool)));
	connect(checkBoxForever, SIGNAL(toggled(bool)), checkBoxSession, SLOT(setChecked(bool)));
	connect(this, SIGNAL(finished(int)), this, SLOT(setAnswer(int)));
}

bool QuestionBox::question(QWidget *parent, const QString &title, const QString &question, const QString &settingsKey)
{
	return QuestionBox(title, question, settingsKey, parent).exec() == Yes;
}

void QuestionBox::setVisible(bool visible)
{
	QMessageBox::setVisible(visible);
	if (visible)
		QTimer::singleShot(0, this, SLOT(checkAnswer()));
}

void QuestionBox::checkAnswer()
{
	if (SETTINGS->settings().contains(settingsKey))
		done(SETTINGS->settings().value(settingsKey).toBool() ? Yes : No);
	else if (answers.contains(settingsKey))
		done(answers[settingsKey] ? Yes : No);
}

void QuestionBox::setAnswer(int result)
{
	if (checkBoxForever->isChecked())
		SETTINGS->settings().setValue(settingsKey, result == Yes);
	else if (checkBoxSession->isChecked())
		answers[settingsKey] = result == Yes;
}
