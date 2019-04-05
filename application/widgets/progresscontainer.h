#ifndef PROGRESSCONTAINER_H
#define PROGRESSCONTAINER_H

#include <QList>
#include <QObject>

class QProgressBar;
class ProgressContainerBase;

class ProgressItemBase : public QObject
{
	Q_OBJECT

public:
	ProgressItemBase(ProgressContainerBase *container, qreal total = 1.0, QProgressBar *progressBar = nullptr);
	virtual ~ProgressItemBase();

	qreal progress() const;
	qreal total() const;
	void setTotal(qreal total);
	void setFormat(QString format);
	bool isProgressComplete() const;

public slots:
	void setProgress(qreal progress);
	void setProgressScaleToOne(qint64 progress, qint64 total);

private:
	ProgressContainerBase * const m_container;
	qreal m_progress, m_total;
	QProgressBar * const m_progressBar;
};

template <class T>
class ProgressItem : public ProgressItemBase
{

public:
	typedef typename T::State State;

	ProgressItem(ProgressContainerBase *container, T *object, qreal total = 1.0, QProgressBar *progressBar = nullptr):
		ProgressItemBase(container, total, progressBar),
		state(State(0)),
		m_object(object)
	{}

	T *object() const
	{
		return m_object;
	}

	State state;

private:
	T *m_object;
};

class ProgressContainerBase
{
protected:
	ProgressContainerBase();
//	ProgressContainerBase(QProgressBar *progressBar);
public:
	virtual ~ProgressContainerBase();
	void update();
	virtual qreal progress() const = 0;
	virtual qreal total() const = 0;
	void setFormat(QString format);
	void setProgressBar(QProgressBar *progressBar);
	void setExtraTotal(qreal total);
	void increaseExtraProgress(qreal progress);

protected:
	QProgressBar *m_progressBar;
	qreal m_progress, m_total;

};

template <class T>
class ProgressContainer : public QList<ProgressItem<T> *>, public ProgressContainerBase
{
public:
	typedef typename ProgressItem<T>::State State;
	typedef ProgressItem<T> Item;
	typedef QList<ProgressItem<T> *> List;

//	ProgressContainer(QProgressBar *progressBar):
//		ProgressContainerBase(progressBar)
//	{}
	~ProgressContainer()
	{
		qDeleteAll(*this);
	}

	virtual qreal progress() const
	{
		qreal progress = m_progress;
		for (int i = 0; i < List::size(); ++i)
			progress += List::at(i)->progress();
		return progress;
	}

	virtual qreal total() const
	{
		qreal total = m_total;
		for (int i = 0; i < List::size(); ++i)
			total += List::at(i)->total();
		return total;
	}

	void append(T *t, qreal total = 1.0, QProgressBar *progressBar = nullptr)
	{
		List::append(new ProgressItem<T>(this, t, total, progressBar));
		update();
	}

	using List::first;
	ProgressItem<T> *first(State state) const
	{
		for (int i = 0; i < List::size(); ++i)
			if (List::at(i)->state == state)
				return List::at(i);
		return nullptr;
	}

	ProgressItem<T> *firstAtBest(State state) const
	{
		for (int i = 0; i < List::size(); ++i)
			if (List::at(i)->state <= state)
				return List::at(i);
		return nullptr;
	}

	using List::last;
	ProgressItem<T> *last(State state) const
	{
		for (int i = List::size() - 1; i >= 0; --i)
			if (List::at(i)->state == state)
				return List::at(i);
		return nullptr;
	}

	bool contains(State state) const
	{
		return first(state);
	}

	bool all(State state) const
	{
		for (int i = 0; i < List::size(); ++i)
			if (List::at(i)->state != state)
				return false;
		return true;
	}

//	bool allAtLeast(State state) const
//	{
//		for (int i = 0; i < List::size(); ++i)
//			if (item->state < state)
//				return false;
//		return true;
//	}

};

//#include "progresscontainer.cpp"

#endif // PROGRESSCONTAINER_H
