// ServerManager.h

#ifndef _SERVERMANAGER_h
#define _SERVERMANAGER_h

#include <Arduino.h>

#include "../../Config/config_master.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#if COMPILE_SERVER
#include <ESPAsyncWebServer.h>
#endif	/*COMPILE_SERVER*/


namespace Network {
	namespace Server
	{
#if COMPILE_SERVER || COMPILE_FTP
		void Initialize();
		void StartServer();
		void StopServer();
		void Deinitialize();
		void StartDnsServer();
		void StopDnsServer();
		void Loop();
#endif



	}
}

#endif

