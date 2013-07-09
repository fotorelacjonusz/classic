#include "abstractuploader.h"

#include <QStringList>
#include <QInputDialog>
#include <QLayout>
#include <QFormLayout>
#include <QDebug>
#include <QDateTime>

AbstractUploader::AbstractUploader(QWidget *parent, QSettings &settings):
	QWidget(parent),
	settings(settings),
	manager(settings)
{
}

AbstractUploader::~AbstractUploader()
{
}

void AbstractUploader::load()
{
	manager.load();
}

void AbstractUploader::save()
{
	manager.save();
}

bool AbstractUploader::init(int imageNumber)
{
	Q_UNUSED(imageNumber);
	return true;
}

void AbstractUploader::abort()
{
	emit abortRequested();
}

void AbstractUploader::finalize()
{
}

QWidget *AbstractUploader::lastInputWidget() const
{
	if (!layout() || layout()->isEmpty())
		return 0;

	QWidget *last = 0;
	for (int i = layout()->count() - 1; i >= 0 && (last = layout()->itemAt(i)->widget()) && last->inherits("QLabel"); --i);
	return last;
}

QString AbstractUploader::lastError() const
{
	return error;
}

QString AbstractUploader::tosUrl() const
{
	return "";
}

QString AbstractUploader::generateFolderName()
{
	return tr("Fotorelacja_") + QDateTime::currentDateTime().toString(Qt::ISODate);
}

QString AbstractUploader::removeAccents(QString diacritical)
{
	static const QString diacriticals = "ŧþðđŋħĸÐŁØ¥łøðßμŒ œ Æ æ ";
	static const QString replacements = "tbodnnkDLOYloosuOEoeAEae";
	
	QString result;
	foreach (QChar c, diacritical.normalized(QString::NormalizationForm_KD))
	{
		if (c.category() > QChar::Mark_Enclosing)
		{
			if ((int)c.toAscii() >= 0)
				result.append(c);
			else
			{
//				qDebug() << c << c.cell() << c.row() << c.unicode() << (int)c.toAscii() << (int)c.toLatin1() << c.decomposition().length();
				int pos = diacriticals.indexOf(c);
				if (pos >= 0)
				{
					result.append(replacements[pos]);
					if (diacriticals[pos + 1] == ' ')
						result.append(replacements[pos + 1]);
				}
			}				
		}
	}
		
	return result;
}

QString AbstractUploader::queryPassword(QString sourceDsc, bool *ok)
{
	if (!tempPassword.isEmpty())
		return tempPassword;
	return tempPassword = QInputDialog::getText(this, tr("Podaj hasło"), tr("Podaj hasło do %1").arg(sourceDsc), QLineEdit::Password, QString(), ok);
}

void AbstractUploader::fixLayout(QFormLayout *child)
{
	QFormLayout *parent = dynamic_cast<QFormLayout *>(this->layout());
	if (!parent)
	{
		qDebug() << tr("Parent layout nie jest typu QFormLayout");
		return;
	}
	int parentRows = parent->rowCount();
	for (int row = 0; row < child->rowCount(); ++row)
	{
		for (QFormLayout::ItemRole role = QFormLayout::LabelRole; role <= QFormLayout::SpanningRole; role = QFormLayout::ItemRole(role + 1))
			if (child->itemAt(row, role))
				parent->setWidget(parentRows + row, role, child->itemAt(row, role)->widget());
	}
}

