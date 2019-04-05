#ifndef OVERLAYLIST_H
#define OVERLAYLIST_H

#include "downloaders/abstractmapdownloader.h"
#include <QListWidget>
#include <QDir>

class Overlay;

class OverlayList : public QListWidget, public AbstractMapDownloader
{
	Q_OBJECT

public:
	explicit OverlayList(QWidget *parent = nullptr);
	virtual ~OverlayList();
	bool makeMap(GeoMap *map);
	DistancePair bestDistance(GeoMap *map) const;

public slots:
	void openFolder() const;

protected:
	bool isOverlayExtension(QString filePath) const;
	bool addOverlayFile(QString filePath);

	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void dragMoveEvent(QDragMoveEvent* event);
	void dragLeaveEvent(QDragLeaveEvent* event);

	QList<Overlay *> overlays;
	QDir overlaysDir;
};

#endif // OVERLAYLIST_H
