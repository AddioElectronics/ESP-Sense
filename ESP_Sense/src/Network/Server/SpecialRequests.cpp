#include "SpecialRequests.h"

#include <AsyncJson.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>


#include <WiFi.h>

#include "../WifiManager.h"

#include "../../Config/ConfigManager.h"
#include "../../../ESP_Sense.h"
#include "../Website/WebStrings.h"
#include "../../GlobalDefs.h"
#include "../../macros.h"
#include "../../JsonHelper.h"
#include "../Website/WebStrings.h"
#include "../Website/WebpageServer.h"
#include "Authentication.h"
#include "../../MQTT/Devices/MqttDeviceManager.h"
#include "../../CrcStream.h"

using namespace Network::Website::Strings;

extern Config_t config;
extern GlobalStatus_t status;

extern HardwareSerial* serial;			//Message
extern HardwareSerial* serialDebug;		//Debug

#if COMPILE_SERVER
extern AsyncWebServer server;
#endif	/*COMPILE_SERVER*/

/// <summary>
/// Shared response string.
/// </summary>
String responseString;

//void ResponseDeviceStatus(AsyncWebServerRequest* request);
//void ResponseMqttDeviceInfo(AsyncWebServerRequest* request);

void Network::Server::SpecialRequests::Initialize()
{
	if (status.server.specialRequestsConfigured) return;

	DEBUG_LOG_LN("Creating Special Requests Handlers...");

	//Can poll to see if server is alive and running. 
	//After updating or hitting reset button in browser,
	//browser will poll this every n seconds to see when server has started again,
	//in which it will restart the page.
	server.on(Website::Strings::Urls::requestAlive, HTTP_GET, [](AsyncWebServerRequest* request) {
		if (status.server.enabled)
			request->send(200);
		else
			request->send(503, ContentType::textPlain, Website::Strings::Messages::serviceUnavailable);
	});

	//Serialize status and send in response.
	/*server.on("/status", HTTP_GET, ResponseDeviceStatus);*/
	server.on(Website::Strings::Urls::requestStatus, HTTP_GET, [](AsyncWebServerRequest* request) {
		//ResponseSerializedData((PACK_JSON_DOC_FUNC)Config::Status::SerializeDeviceStatus, request);
		ResponseSerializedData("status", 3072, (JsonHelper::PACK_JSON_FUNC)Config::Status::PackDeviceStatus, request);
	});

	//Serialize MQTT device info and send in response.
	server.on(Website::Strings::Urls::requestMqttDeviceInfo, HTTP_GET, [](AsyncWebServerRequest* request) {
		//ResponseSerializedData((PACK_JSON_DOC_FUNC)Mqtt::DeviceManager::GetJsonDeviceInfo, request);

		if (!status.mqtt.devicesConfigured)
		{
			request->send(503, ContentType::textPlain, Website::Strings::Messages::serviceUnavailable);
			return;
		}

		ResponseSerializedData("mqttDeviceInfo", 2048, (JsonHelper::PACK_JSON_FUNC)Mqtt::DeviceManager::PackDeviceInfo, request);
	});

	//Get ESP Sense Version
	server.on(Website::Strings::Urls::requestVersion, HTTP_GET, [](AsyncWebServerRequest* request) {
		ResponseSerializedData("version", 2048, (JsonHelper::PACK_JSON_FUNC)[](JsonVariant& doc) {
			doc["version"].set(status.misc.version);
			doc["configMode"].set(status.device.configMode);
			return 0;
		}, request, true, false);
	});

	//Restart
	server.on(Website::Strings::Urls::requestReset, HTTP_POST, [](AsyncWebServerRequest* request) {
		if (!Server::Authentication::IsAuthenticated(request))
		{
			request->send(511, ContentType::textPlain, Messages::networkAuthenticationRequried);
			return;
		}

		request->send(200);
		WifiManager::Disconnect();
		delay(1000);
		EspSense::RestartDevice();
	});

	//Control
	server.on(Website::Strings::Urls::requestControl, HTTP_POST, [](AsyncWebServerRequest* request) {
		request->send(501, ContentType::textPlain, Website::Strings::Messages::notImplemented);
		return;
		
		if (!Server::Authentication::IsAuthenticated(request))
		{
			request->send(511, ContentType::textPlain, Messages::networkAuthenticationRequried);
			return;
		}

		/*Commands*/
		//Enable backups
		//Set test config to default path
		//Cancel firmware rollback?
		//Restart?
		//Set new username and password


		request->send(200);
		WifiManager::Disconnect();
		delay(1000);
		EspSense::RestartDevice();
	});

	DEBUG_LOG_LN("...Special Request Handlers Created.");
	status.server.specialRequestsConfigured = true;
}



/// <summary>
/// Calls a function which serializes data and sends as a response stream.
/// </summary>
/// <param name="docSize">Size to allocate for the document</param>
/// <param name="packFunc">Function pointer to a method that packs data in to a JSON document.</param>
/// <param name="request">Web request</param>
/// <param name="respondOnError">If the JSON data was unable to be sent, do you want an error code to be sent?</param>
/// <returns>Recommended HTTP error response code. If returns 200, a respone has already been sent.</returns>
int Network::Server::SpecialRequests::ResponseSerializedData(const char* rootName, size_t docSize, JsonHelper::PACK_JSON_FUNC packFunc, AsyncWebServerRequest* request, bool packArray, bool requireAuth, bool respondOnError, bool messagedResponse)
{
	DEBUG_LOG_LN("ResponseSerializedData");
	if (requireAuth)
		if (!Network::Server::Authentication::IsAuthenticated(request))
		{
			if (respondOnError)
				request->send(511, ContentType::textPlain, Messages::networkAuthenticationRequried);
				//Server::SendHttpResponseCode(511, request, messagedResponse);
			return 511;
		}

	DynamicJsonDocument* doc = JsonHelper::CreateAndPackDocument(rootName, docSize, packFunc, packArray);

	if (doc != nullptr) {

		DEBUG_LOG_LN("request->beginResponseStream");
		//Serialize document in response stream.
		AsyncResponseStream* response = request->beginResponseStream(ContentType::appJSON);
		response->addHeader(Headers::defaultHeaderName, Headers::defaultHeaderValue);
		response->setCode(200);
		serializeJson(*doc, *response);
		request->send(response);
		delete doc;
		return 200;
	}
	else
	{
		if (respondOnError)
			request->send(500, ContentType::textPlain, Messages::internalServerError);
			//Server::SendHttpResponseCode(500, request, messagedResponse);
		return 500;
	}
}


///// <summary>
///// Calls a function which serializes data and sends as a response.
///// </summary>
//void Network::Server::SpecialRequests::ResponseSerializedData(JsonHelper::PACK_JSON_DOCUMENT_FUNC packFunc, AsyncWebServerRequest* request)
//{
//	if (!Network::Server::Authentication::IsAuthenticated(request)) return;
//	
//	DynamicJsonDocument* doc;
//	size_t size = serializeFunction(&doc);
//
//	if (size) {
//		//request->send(200, ContentType::appJSON, responseString.c_str());
//		//responseString.clear();
//
//		//Serialize document as response stream.
//		AsyncResponseStream* response = request->beginResponseStream(ContentType::appJSON);
//		serializeJson(*doc, *response);
//		request->send(response);
//
//		delete doc;
//	}
//	else
//		request->send(404);
//}

///// <summary>
///// Serialize status and send in response.
///// </summary>
//void ResponseDeviceStatus(AsyncWebServerRequest* request)
//{
//	if (!Network::Server::Authentication::IsAuthenticated(request)) return;
//
//	size_t size = Config::Status::SerializeDeviceStatus(responseString);
//
//	if (size) {
//		request->send(200, Network::Website::Strings::ContentType::appJSON, responseString.c_str());
//		responseString.clear();
//	}
//	else
//		request->send(404);
//}
//
//
//
//void ResponseMqttDeviceInfo(AsyncWebServerRequest* request)
//{
//	if (!Network::Server::Authentication::IsAuthenticated(request)) return;
//
//	size_t size = Mqtt::DeviceManager::GetJsonDeviceInfo(responseString);
//
//	if (size) {
//		request->send(200, Network::Website::Strings::ContentType::appJSON, responseString.c_str());
//	}
//	else
//		request->send(404);
//}