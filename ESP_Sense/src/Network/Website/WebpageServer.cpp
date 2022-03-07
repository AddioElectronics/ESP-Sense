#include "WebpageServer.h"





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


//bool Network::Website::ServeErrorPage(AsyncWebServerRequest* request, int error)
//{
//	char* path;

//	int size = asprintf(&path, Strings::Paths::errorPattern, error);

//	if (size > 0)
//	{
//		if (ESP_FS.exists(path))
//			Server::ServeWebpage(path, request);
//		else
//		{
//			size = 0;	//Return false
//			request->send(error);
//		}

//		free(path);
//		return size;
//	}

//	request->send(error);
//	return false;
//}

