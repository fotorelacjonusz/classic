#include "progresscontainer.h"

#include <QProgressBar>
//#include <QDebug>

#define MULTIPLIER 10000

ProgressItemBase::ProgressItemBase(ProgressContainerBase *container, qreal total, QProgressBar *progressBar):
	m_container(container),
	m_progress(0.0),
	m_total(total),
	m_progressBar(progressBar)
{
	setTotal(total);
}

ProgressItemBase::~ProgressItemBase()
{
}

qreal ProgressItemBase::progress() const
{
	return m_progress;
}

void ProgressItemBase::setProgress(qreal progress)
{
	m_progress = progress;
	if (m_progressBar)
		m_progressBar->setValue(qMin(qRound(progress * MULTIPLIER), m_progressBar->maximum()));
	m_container->update();
}

qreal ProgressItemBase::total() const
{
	return m_total;
}

void ProgressItemBase::setTotal(qreal total)
{
	m_total = total;
	if (m_progressBar)
		m_progressBar->setMaximum(total * MULTIPLIER);
	m_container->update();
}

void ProgressItemBase::setFormat(QString format)
{
	if (m_progressBar)
		m_progressBar->setFormat(format);
}

bool ProgressItemBase::isProgressComplete() const
{
	return m_progress >= m_total;
}

void ProgressItemBase::setProgressScaleToOne(qint64 progress, qint64 total)
{
	if (!qFuzzyCompare(m_total, 1.0))
		setTotal(1.0);

	setProgress(qreal(progress) / total);
}

ProgressContainerBase::ProgressContainerBase():
	m_progressBar(0), m_progress(0.0), m_total(0.0)
{
}

//ProgressContainerBase::ProgressContainerBase(QProgressBar *progressBar):
//	m_progressBar(progressBar)
//{
//}

ProgressContainerBase::~ProgressContainerBase()
{
}

void ProgressContainerBase::update()
{
	m_progressBar->setMaximum(total() * MULTIPLIER);
	m_progressBar->setValue(qMin(qRound(progress() * MULTIPLIER), m_progressBar->maximum()));
}

void ProgressContainerBase::setFormat(QString format)
{
	m_progressBar->setFormat(format);
}

void ProgressContainerBase::setProgressBar(QProgressBar *progressBar)
{
	m_progressBar = progressBar;
}

void ProgressContainerBase::setExtraTotal(qreal total)
{
	m_total = total;
	update();
}

void ProgressContainerBase::increaseExtraProgress(qreal progress)
{
	m_progress += progress;
	update();
}

