#include "embeddedjavascript.h"

#include <QFile>
#include <QFileInfo>
#include <QWebEngineProfile>
#include <QWebEngineScriptCollection>
#include <QtCore/QStringBuilder>

EmbeddedJavascript::EmbeddedJavascript(QString scriptSourcePath)
{
	QFile sourceFile(scriptSourcePath);
	QFileInfo scriptInfo(sourceFile);

	sourceFile.open(QIODevice::ReadOnly);
	setSourceCode(sourceFile.readAll());
	setName("Fotorelacjonusz: " % scriptInfo.baseName());
	setWorldId(worldId);
}

EmbeddedJavascript::~EmbeddedJavascript()
{
}

/**
 * @brief Reads script source from given path, and uploads it to the browser's
 * default profile.
 * @param scriptSourcePath QString resource path to script source
 */
void EmbeddedJavascript::insertIntoProfile(QString scriptName)
{
	auto profile = QWebEngineProfile::defaultProfile();
	EmbeddedJavascript script(scriptName);
	profile->scripts()->insert(script);
}
