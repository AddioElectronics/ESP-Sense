#ifndef _SPECIALREQUESTS_h
#define _SPECIALREQUESTS_h

#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "../../Config/config_master.h"
#include "../../Config/global_status.h"
#include "../../Config/ConfigManager.h"
#include "../../JsonHelper.h"


#if COMPILE_SERVER
#include <ESPAsyncWebServer.h>
#endif	/*COMPILE_SERVER*/

///// <summary>
///// Function which serializes a specific document into a string.
///// </summary>
//typedef size_t (*SERIALIZE_DATA_FUNC)(String* serializeTo, DynamicJsonDocument** out_doc);
//
///// <summary>
///// Function which prepares a specific JSON document for serializing in to a stream.
///// </summary>
//typedef size_t (*PACK_JSON_DOC_FUNC)(DynamicJsonDocument** out_doc);

namespace Network
{
	namespace Server
	{
		namespace SpecialRequests
		{

			void Initialize();
			// void Network::Server::SpecialRequests::ResponseSerializedData(PACK_JSON_DOC_FUNC serializeFunction, AsyncWebServerRequest * request)
			//void ResponseSerializedData(PACK_JSON_DOC_FUNC serializeFunction, AsyncWebServerRequest* request);
			int ResponseSerializedData(const char* rootName, size_t docSize, JsonHelper::PACK_JSON_FUNC packFunc,  AsyncWebServerRequest* request, bool packArray = false, bool requireAuth = true, bool respondOnError = true, bool messagedResponse = true);
		}
	}
}
#endif

