/*
	May switch to ESPAsyncWebServers built in authentication.
	Was using non async webserver before, and needed more control than the way arduino Basic HTTP authentication with WebServer could provide.
*/

#ifndef _AUTHENTICATION_h
#define _AUTHENTICATION_h

#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "../../Config/config_master.h"


#if COMPILE_SERVER
#include <ESPAsyncWebServer.h>
#endif	/*COMPILE_SERVER*/

namespace Network
{
	namespace Server
	{
		namespace Authentication
		{
			void Initialize();

			bool IsAuthenticated(IPAddress& remoteIp);
			bool IsAuthenticated(AsyncWebServerRequest* request);
			void Logout();
		}
	}
}
#endif

