#ifndef EMBEDDEDJAVASCRIPT_H
#define EMBEDDEDJAVASCRIPT_H

#include <QWebEngineScript>

/**
 * @brief Easy way to inject client-side scripts to embedded Chromium browser.
 *
 * A convenience QWebEngineScript subclass which is responsible for uploading
 * JavaScript files from Qt resource collections (*.qrc) to the default profile
 * of embedded browser.
 *
 * Example usage:
 *
 * @code
 * EmbeddedJavascript::insertIntoProfile(":/src/web/ssc.js");
 * @endcode
 *
 * @note By project's convention, scripts should be stored in /src/web
 * directory.
 */
class EmbeddedJavascript : public QWebEngineScript
{

public:
	explicit EmbeddedJavascript(QString scriptName);
	EmbeddedJavascript(const EmbeddedJavascript&) = delete;
	virtual ~EmbeddedJavascript();

	static void insertIntoProfile(QString scriptName);

	/// Browser's world ID in which scripts managed by this class are going
	/// to be executed.
	static const auto worldId = QWebEngineScript::ApplicationWorld;
};

#endif // EMBEDDEDJAVASCRIPT_H
