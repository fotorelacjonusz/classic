#include "imgurresponse.h"
#include "networktransaction.h"

#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QStringList>
#include <QDebug>

ImgurResponse::ImgurResponse(const NetworkTransaction &tr):
	success(false),
	status(0)
{
	QScriptEngine se;
	QScriptValue val = se.evaluate("(" + tr.data + ")");

	QScriptValueIterator i(val);
	while (i.hasNext())
	{
		i.next();
		if (i.name() == "data")
		{
			QScriptValueIterator j(i.value());
			while (j.hasNext())
			{
				j.next();
				data.insert(j.name(), j.value().toString());
			}
		}
		else if (i.name() == "error")
			error = i.value().toString();
		else if (i.name() == "success")
			success = i.value().toBool();
		else if (i.name() == "status")
			status = i.value().toInt32();
		else
			insert(i.name(), i.value().toString());
	}
	if (data.contains("error") && error.isEmpty())
		error = data.take("error");

	QStringList errors;
	if (!tr.error.isEmpty())
		errors << tr.error;
	if (!error.isEmpty())
		errors << error;
	if (contains("message"))
		errors << value("message");
	if (data.contains("message"))
		errors << data["message"];

	mergedError = errors.join("\n");
}

void ImgurResponse::debug() const
{
	QDebug debug(QtDebugMsg);
	debug.nospace() << "{\n";
	debug << "  success:     " << success << "\n";
	debug << "  status:      " << status << "\n";
	debug << "  error:       " << error << "\n";
	debug << "  mergedError: " << mergedError << "\n";
	debug << "  data:\n";
	debug << "  {\n";
	for (ParamMap::ConstIterator i = data.begin(); i != data.end(); ++i)
		debug << "    " << i.key() << ":\t" << i.value() << "\n";
	debug << "  }\n";
	for (ParamMap::ConstIterator i = begin(); i != end(); ++i)
		debug << "  " << i.key() << ":\t" << i.value() << "\n";
	debug << "}\n";
	debug.space();
}
