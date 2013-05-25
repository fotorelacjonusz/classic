#ifndef RECENTTHREADSMENU_H
#define RECENTTHREADSMENU_H

#include <QMenu>
class QSettings;
class QActionGroup;
class QAction;

class RecentThreadsMenu : public QMenu
{
	Q_OBJECT
public:
	explicit RecentThreadsMenu(QSettings &settings, QWidget *parent = 0);
	virtual ~RecentThreadsMenu();
	
protected:
	QAction *addUpdateAction(QString threadId);
	
public slots:
	void threadPosted(QString threadId, QString threadTitle, int imageNumber);
	void unselect();
	
private slots:
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
