#pragma once

#include <QString>
#include <QMap>

class NetworkTransaction;

/**
 * @brief Parsed JSON responses from Imgur.
 *
 * Imgur API returns JSON objects, which conform following schema:
 *
 *   {
 *     "success": <bool, whether the call has succeeded>,
 *     "status": <integer number equal to HTTP status code of the response>,
 *     "data": <object with more details>
 *   }
 *
 * The "data" is always an object and contains more details about outcome of
 * given operation, most notably "error", which contains error message
 * in case of error responses.
 *
 * Call debug() to print the response in human-readable way in development.
 */
class ImgurResponse
{
	void parseResponse(const NetworkTransaction &tr);

public:
	ImgurResponse(const NetworkTransaction &tr);
	void debug() const;

	/// @brief Key-value pairs found in "data" object of server's response.
	/// @todo Consider making it <QString, QVariant> to avoid type conversions.
	QMap<QString, QString> data;
	bool success;
	int status;
	QString error;
	QString mergedError;
};
