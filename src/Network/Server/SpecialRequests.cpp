#include "SpecialRequests.h"

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include <WiFi.h>

#include "../WifiManager.h"

#include "../../Config/ConfigManager.h"
#include "../../../ESP_Sense.h"
#include "../Website/WebStrings.h"
#include "../../GlobalDefs.h"
#include "../../macros.h"
#include "../Website/WebStrings.h"
#include "../Website/WebpageServer.h"
#include "Authentication.h"

extern Config_t config;
extern DeviceStatus_t status;

#if COMPILE_SERVER
extern AsyncWebServer server;
#endif	/*COMPILE_SERVER*/

/// <summary>
/// Shared response string.
/// </summary>
String responseString;

void ResponseDeviceStatus(AsyncWebServerRequest* request);

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
			request->send(503);
	});

	//Serialize status and send in response.
	server.on("/status", HTTP_GET, ResponseDeviceStatus);

	//Restart
	server.on(Website::Strings::Urls::requestReset, HTTP_POST, [](AsyncWebServerRequest* request) {
		if (!Server::Authentication::IsAuthenticated(request))
		{
			request->send(401);
			return;
		}

		request->send(200);
		WifiManager::Disconnect();
		delay(1000);
		EspSense::RestartDevice();
	});

	DEBUG_LOG_LN("...Special Request Handlers Created.");
	status.server.specialRequestsConfigured = true;
}


/// <summary>
/// Serialize status and send in response.
/// </summary>
void ResponseDeviceStatus(AsyncWebServerRequest* request)
{
	if (!Network::Server::Authentication::IsAuthenticated(request)) return;

	size_t size = Config::Status::SerializeDeviceStatus(responseString);

	if (size) {
		request->send(200, Network::Website::Strings::ContentType::appJSON, responseString.c_str());
		responseString.clear();
	}
	else
		request->send(404);
}