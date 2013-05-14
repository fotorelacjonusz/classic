#include "overlaylist.h"
#include "overlay.h"
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
	foreach (QFileInfo kmzInfo, overlaysDir.entryInfoList(QStringList() << "*.kmz" << "*.kmr", QDir::Readable | QDir::Files, QDir::Time | QDir::Reversed))
		addOverlayFile(kmzInfo.absoluteFilePath());
}

OverlayList::~OverlayList()
{
	qDeleteAll(overlays);
}

bool OverlayList::makeMap(GeoMap *map)
{
	foreach (Overlay *overlay, overlays)
		if (overlay->makeMap(map))
			return true;
	return false;
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
	static const QIcon ok = QIcon(":res/dialog-ok-apply.png");
	static const QIcon noOk = QIcon(":res/edit-delete.png");

	Overlay *overlay = new Overlay(filePath);
	overlays.append(overlay);
	addItem(new QListWidgetItem(overlay->isValid() ? ok : noOk, overlay->toString(), this));
	return overlay->isValid();
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
