#include "WebpageServer.h"

#include "WebStrings.h"
#include "../../macros.h"



String Network::Server::GeneratePathFromURL(const char* url)
{
	String path = url;
	if (!path.startsWith(Website::Strings::Paths::root))
		path = Website::Strings::Paths::root + path;

	return path;
}

bool Network::Server::ServeWebpage(String& path, AsyncWebServerRequest* request)
{
	if (!path.startsWith(Website::Strings::Paths::root))
		path = Website::Strings::Paths::root + path;

	if (!ESP_FS.exists(path))
		return false;

	request->send(ESP_FS, path);

	return true;
}

bool Network::Server::ServeWebpage(const char* path, AsyncWebServerRequest* request)
{
	String spath = path;
	return ServeWebpage(spath, request);
}

bool Network::Server::Server401(AsyncWebServerRequest* request)
{
	return ServeWebpage(Network::Website::Strings::Urls::error401, request);
}

bool Network::Server::Server404(AsyncWebServerRequest* request)
{
	return ServeWebpage(Network::Website::Strings::Urls::error404, request);
}

bool Network::Server::Server501(AsyncWebServerRequest* request)
{
	return ServeWebpage(Network::Website::Strings::Urls::error501, request);
}

