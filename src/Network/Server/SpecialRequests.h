#ifndef _SPECIALREQUESTS_h
#define _SPECIALREQUESTS_h

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
		namespace SpecialRequests
		{

			void Initialize();

		}
	}
}
#endif

