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

		///// <summary>
		///// Sends a coded response to the server, with a plain text message if sendMessage is true, and message is available.
		///// </summary>
		///// <param name="code">HTTP response code.</param>
		///// <param name="request">Server request to respond to.</param>
		///// <param name="message">Will send a plain text message if available.</param>
		//inline void SendHttpResponseCode(int code, AsyncWebServerRequest* request, bool sendMessage = true)
		//{
		//	const char* message = nullptr;

		//	switch (code)
		//	{
		//	case 401:
		//		message = Website::Strings::Messages::unauthorized;
		//		break;
		//	case 500:
		//		message = Website::Strings::Messages::internalServerError;
		//		break;
		//	case 511:
		//		message = Website::Strings::Messages::networkAuthenticationRequried;
		//		break;
		//	}

		//	if (message != nullptr && sendMessage)
		//		request->send(code, Website::Strings::ContentType::textPlain, message);
		//	else
		//		request->send(code);
		//}

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

		inline bool Server511(AsyncWebServerRequest* request)
		{
			return ServeWebpage(Network::Website::Strings::Urls::error511, request);
		}
	}
}
#endif

