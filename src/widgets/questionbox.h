#ifndef QUESTIONBOX_H
#define QUESTIONBOX_H

#include <QMessageBox>

class QCheckBox;

class QuestionBox : public QMessageBox
{
	Q_OBJECT
public:
	explicit QuestionBox(const QString &title, const QString &question, const QString &settingsKey, QWidget *parent = 0);
	static bool question(QWidget *parent, const QString &title, const QString &question, const QString &settingsKey);

	virtual void setVisible(bool visible);

private slots:
	void checkAnswer();
	void setAnswer(int result);

private:
	const QString settingsKey;

	QCheckBox *checkBoxForever;
	QCheckBox *checkBoxSession;

	static QMap<QString, bool> answers;
};

#endif // QUESTIONBOX_H
