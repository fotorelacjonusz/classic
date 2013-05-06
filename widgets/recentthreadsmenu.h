#ifndef RECENTTHREADSMENU_H
#define RECENTTHREADSMENU_H

#include <QMenu>
#include <QSettings>
class QActionGroup;
class QAction;

class RecentThreadsMenu : public QMenu
{
	Q_OBJECT
public:
	explicit RecentThreadsMenu(QSettings &settings, QWidget *parent = 0);
	virtual ~RecentThreadsMenu();
	
//	QString selectedThread() const;
//	QPair<QString, int> selectedThread() const;
	
protected:
	QAction *addUpdateAction(QString threadId);
	
public slots:
	void imagePosted(QString threadId, QString threadTitle, int imageNumber);
//	void postingFinished();
	
private slots:
	void noneTriggered();
	void actionTriggered(bool checked);
	
protected:
	QSettings &settings;
	QVariantHash threadTitles;
	QVariantHash imageNumbers;
	QHash<QString, QAction *> actions;
	QActionGroup *actionGroup;
	QAction *firstAction;
};

#endif // RECENTTHREADSMENU_H
