#include "overlaylist.h"
#include "overlay.h"
#include "downloaders/geomap.h"
//#include "exception.h"
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QUrl>
#include <QDesktopServices>
#include <QDebug>

#define FOTORELACJONUSZ_DIR_CSTR ".fotorelacjonusz"

OverlayList::OverlayList(QWidget *parent) :
	QListWidget(parent)
{
	if (!overlaysDir.cd("./overlays"))
	{
		overlaysDir = QDir::home();
		overlaysDir.mkdir(FOTORELACJONUSZ_DIR_CSTR);
		overlaysDir.cd(FOTORELACJONUSZ_DIR_CSTR);
	}
	foreach (QFileInfo fileInfo, overlaysDir.entryInfoList(QStringList() << "*.kmz" << "*.kmr", QDir::Readable | QDir::Files, QDir::Time | QDir::Reversed))
		addOverlayFile(fileInfo.absoluteFilePath());
}

OverlayList::~OverlayList()
{
	qDeleteAll(overlays);
}

bool OverlayList::makeMap(GeoMap *map)
{
	if (!map->isCommon)
	{
		DistancePair best = bestDistance(map);
		if (best.first && best.first->makeMap(map))
			return true;
	}
	
	foreach (Overlay *overlay, overlays)
		if (overlay->makeMap(map))
			return true;
	return false;
}

OverlayList::DistancePair OverlayList::bestDistance(GeoMap *map) const
{
	DistancePair best = qMakePair((OverlayImage *)0, 10000.0);
	foreach (Overlay *overlay, overlays)
	{
		DistancePair current = overlay->bestDistance(map);
		if (current.second < best.second)
			best = current;
	}
	return best;
}

void OverlayList::openFolder() const
{
	QDesktopServices::openUrl(QUrl("file:///" + QDir::toNativeSeparators(overlaysDir.absolutePath())));
}

bool OverlayList::isOverlayExtension(QString filePath) const
{
	return (QStringList() << "kmz" << "kmr").contains(filePath.section(".", -1).toLower());
}

bool OverlayList::addOverlayFile(QString filePath)
{
//	static const QIcon ok = QIcon(":res/dialog-ok-apply.png");
//	static const QIcon noOk = QIcon(":res/edit-delete.png");
	
//	QString error;
//	Overlay *overlay;
	try
	{
		Overlay *overlay = new Overlay(filePath);
		overlays.append(overlay);
		addItem(new QListWidgetItem(QIcon(":res/dialog-ok-apply.png"), overlay->toString(), this));
		return true;
	}
	catch (const Exception &e)
	{
		addItem(new QListWidgetItem(QIcon(":res/edit-delete.png"), filePath.split('/').last() + '\t' + e.message(), this));
		return false;
	}
}

void OverlayList::dragEnterEvent(QDragEnterEvent *event)
{
	foreach (QUrl url, event->mimeData()->urls())
	{
		if (url.scheme() == "file" && isOverlayExtension(url.toLocalFile()))
		{
			event->acceptProposedAction();
			return;
		}
	}
}

void OverlayList::dropEvent(QDropEvent *event)
{
	foreach (QUrl url, event->mimeData()->urls())
		if (url.scheme() == "file" && isOverlayExtension(url.toLocalFile()))
		{
			event->acceptProposedAction();
			QString newFilePath = overlaysDir.absolutePath() + "/" + QFileInfo(url.toLocalFile()).fileName();
			QFile::copy(url.toLocalFile(), newFilePath);
			addOverlayFile(newFilePath);
		}
}

void OverlayList::dragMoveEvent(QDragMoveEvent *event)
{
	QWidget::dragMoveEvent(event);
}

void OverlayList::dragLeaveEvent(QDragLeaveEvent *event)
{
	QWidget::dragLeaveEvent(event);
}
