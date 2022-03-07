#ifndef _WEBPAGESERVER_h
#define _WEBPAGESERVER_h

#include <arduino.h>

#include <ESPAsyncWebServer.h>

#include "WebStrings.h"
#include "../../macros.h"

namespace Network {
	namespace Server {
		String GeneratePathFromURL(const char* url);
		bool ServeWebpage(String& path, AsyncWebServerRequest* request);

		inline bool ServeWebpage(const char* path, AsyncWebServerRequest* request)
		{
			String spath = path;
			return ServeWebpage(spath, request);
		}

		//bool ServeErrorPage(AsyncWebServerRequest* request, int error);

		inline bool Server401(AsyncWebServerRequest* request)
		{
			return ServeWebpage(Network::Website::Strings::Urls::error401, request);
		}

		inline bool Server404(AsyncWebServerRequest* request)
		{
			return ServeWebpage(Network::Website::Strings::Urls::error404, request);
		}

		inline bool Server501(AsyncWebServerRequest* request)
		{
			return ServeWebpage(Network::Website::Strings::Urls::error501, request);
		}
	}
}
#endif

