#ifndef _WEBPAGESERVER_h
#define _WEBPAGESERVER_h

#include <arduino.h>

#include <ESPAsyncWebServer.h>

namespace Network {
	namespace Server {
		String GeneratePathFromURL(const char* url);
		bool ServeWebpage(String& path, AsyncWebServerRequest* request);
		bool ServeWebpage(const char* path, AsyncWebServerRequest* request);
		bool Server401(AsyncWebServerRequest* request);
		bool Server404(AsyncWebServerRequest* request);
		bool Server501(AsyncWebServerRequest* request);
	}
}
#endif

