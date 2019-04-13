#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QString>
#include <QVariant>
#include <QSet>
#include <QPointer>

class QSettings;
class QButtonGroup;
class QCalendarWidget;
class QCheckBox;
class QComboBox;
class QDateEdit;
class QDateTimeEdit;
class QDial;
class QDoubleSpinBox;
class QFontComboBox;
class QGroupBox;
class QLineEdit;
class QPlainTextEdit;
class QSlider;
class QSpinBox;
class QTextEdit;
class QTimeEdit;
class QWidget;
class QMainWindow;

class SettingsManager
{
	typedef QPair<QPointer<QObject>, QByteArray> Listener;

	class AbstractInput
	{
	protected:
		AbstractInput(QString name, QVariant defaultVal); //QObject *receiver, const char *member,

	public:
		virtual ~AbstractInput();
		virtual QVariant toVariant() const = 0;
		virtual void fromVariant(QVariant variant) = 0;

		void connect(QObject *receiver, const char *member);
		void changedRemotely() const;

		const QString key;
		QSet<Listener> listeners;
//		QObject *const receiver;
//		const char *const member;
		const QVariant defaultVal;
		bool wasChanged;

	protected:
		QByteArray encode(QString pass) const;
		QString decode(QByteArray pass) const;

	private:
		const QByteArray pasKey;
	};

	template <typename T>
	class Input : public AbstractInput
	{
	public:
		Input(QString key, T *object, QVariant defaultVal): //QObject *receiver, const char *member,
			AbstractInput(key, defaultVal), object(object) {} // receiver, member,
		inline QVariant toVariant() const
		{
			return *object;
		}
		inline void fromVariant(QVariant variant)
		{
			*object = variant.value<T>();
		}
	private:
		T *const object;
	};


	template <typename V>
	class AbstractWrapper
	{
	public:
		AbstractWrapper(AbstractInput *input):
			input(input) {}
		virtual ~AbstractWrapper() {}
		virtual V value() const = 0;
		inline virtual bool wasChanged() const
		{
			return input->wasChanged;
		}
		inline AbstractInput *getInput() const
		{
			return input;
		}
	protected:
		AbstractInput *input;
	};

	template <typename V>
	class DirectWrapper : public AbstractWrapper<V>
	{
	public:
		DirectWrapper(AbstractInput *input):
			AbstractWrapper<V>(input) {}
		inline V value() const
		{
			return this->input->toVariant().template value<V>();
		}
	};

	template <typename V, typename O>
	class DelegateWrapper : public AbstractWrapper<V>
	{
	public:
		DelegateWrapper(AbstractInput *input, O *object, V (O::*method)() const):
			AbstractWrapper<V>(input), object(object), method(method) {}
		inline V value() const
		{
			return (object->*method)();
		}
	private:
		O *object;
		V (O::*method)() const;
	};

protected:

	class AbstractField
	{
	public:
		virtual void connect(QObject *receiver, const char *member) = 0;
	};

	template <typename V>
	class Field : public AbstractField
	{
	public:
		Field():
			wrapper(0) {}
		virtual ~Field()
		{
			delete wrapper;
		}
		inline void init(AbstractInput *input)
		{
			wrapper = new DirectWrapper<V>(input);
		}
		template<typename O>
		inline void init(AbstractInput *input, O *object, V (O::*method)() const)
		{
			wrapper = new DelegateWrapper<V, O>(input, object, method);
		}
		inline V v() const
		{
			return wrapper->value();
		}
		inline operator V() const
		{
			return wrapper->value();
		}
		inline void connect(QObject *receiver, const char *member)
		{
			wrapper->getInput()->connect(receiver, member);
		}
		inline void changedRemotely() const
		{
			wrapper->getInput()->changedRemotely();
		}
		inline bool wasChanged() const
		{
			return wrapper->wasChanged();
		}
		inline AbstractInput *getInput() const
		{
			return wrapper->getInput();
		}
	private:
		AbstractWrapper<V> *wrapper;
	};

public:
	SettingsManager(QSettings &settings);
	~SettingsManager();

	/*
	template <typename T>
	inline AbstractInput *makeInput(QString key, T *object, QObject *receiver, const char *member, QVariant defaultVal = QVariant())
	{
		return (inputs << new Input<T>(key, object, receiver, member, defaultVal)).last();
	}
	*/

	template <typename T>
	inline AbstractInput *makeInput(QString key, T *object, QVariant defaultVal = QVariant())
	{
		return (inputs << new Input<T>(key, object, defaultVal)).last(); // 0, 0,
	}

	void connectMany(QObject *receiver, const char *member, AbstractField *f0, AbstractField *f1 = 0,
					 AbstractField *f2 = 0, AbstractField *f3 = 0, AbstractField *f4 = 0, AbstractField *f5 = 0,
					 AbstractField *f6 = 0, AbstractField *f7 = 0, AbstractField *f8 = 0, AbstractField *f9 = 0);

public:
	void load(AbstractInput *singleInput = 0);
	void save(AbstractInput *singleInput = 0);

private:
	QSettings &settings;
	QList<AbstractInput *>inputs;
};

// the resulting array length is the same as a1 lenght
QByteArray operator^(const QByteArray &a1, const QByteArray &a2);

template<>QVariant SettingsManager::Input<QButtonGroup>::toVariant() const;
template<>void     SettingsManager::Input<QButtonGroup>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QCalendarWidget>::toVariant() const;
template<>void     SettingsManager::Input<QCalendarWidget>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QCheckBox>::toVariant() const;
template<>void     SettingsManager::Input<QCheckBox>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QComboBox>::toVariant() const;
template<>void     SettingsManager::Input<QComboBox>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QDateEdit>::toVariant() const;
template<>void     SettingsManager::Input<QDateEdit>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QDateTimeEdit>::toVariant() const;
template<>void     SettingsManager::Input<QDateTimeEdit>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QDial>::toVariant() const;
template<>void     SettingsManager::Input<QDial>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QDoubleSpinBox>::toVariant() const;
template<>void     SettingsManager::Input<QDoubleSpinBox>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QFontComboBox>::toVariant() const;
template<>void     SettingsManager::Input<QFontComboBox>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QGroupBox>::toVariant() const;
template<>void     SettingsManager::Input<QGroupBox>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QLineEdit>::toVariant() const;
template<>void     SettingsManager::Input<QLineEdit>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QPlainTextEdit>::toVariant() const;
template<>void     SettingsManager::Input<QPlainTextEdit>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QSlider>::toVariant() const;
template<>void     SettingsManager::Input<QSlider>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QSpinBox>::toVariant() const;
template<>void     SettingsManager::Input<QSpinBox>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QTextEdit>::toVariant() const;
template<>void     SettingsManager::Input<QTextEdit>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QTimeEdit>::toVariant() const;
template<>void     SettingsManager::Input<QTimeEdit>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QWidget>::toVariant() const;
template<>void     SettingsManager::Input<QWidget>::fromVariant(QVariant variant);
template<>QVariant SettingsManager::Input<QMainWindow>::toVariant() const;
template<>void     SettingsManager::Input<QMainWindow>::fromVariant(QVariant variant);


/*
template<>QVariant SettingsManager::Field<>::toVariant() const;
template<>void     SettingsManager::Field<>::fromVariant(QVariant variant);
*/



#endif // SETTINGSMANAGER_H












