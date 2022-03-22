#include "MqttDeviceWeb.h"

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#include <WiFi.h>


#include "MqttDevice.h"
#include "../../Macros.h"
#include "../../Network/Server/ServerManager.h"
#include "../../Network/Server/Authentication.h"
#include "../../Network/Website/WebStrings.h"

extern GlobalStatus_t status;

extern AsyncWebServer server;

using namespace Network::Website::Strings;


void MqttDeviceWeb::Initialize(MqttDevice* parentDevice)
{
	DEBUG_LOG_F("Initializing Webpage for %s\r\n", parentDevice->name.c_str());



	//memset(&webStatus, 0, sizeof(MqttDeviceWeb::Status_t));

	webStatus.enabled = true;
	_parentDevice = parentDevice;


	MqttDeviceType deviceType = parentDevice->GetDeviceType();

	webStatus.url = "/mqtt/devices/";

	switch (deviceType)
	{
	case MqttDeviceType::MQTT_BINARY_SENSOR:
		webStatus.url += "binarySensors/";
		break;
	case MqttDeviceType::MQTT_SENSOR:
		webStatus.url += "sensors/";
		break;
	case MqttDeviceType::MQTT_BUTTON:
		webStatus.url += "buttons/";
		break;
	case MqttDeviceType::MQTT_LIGHT:
		webStatus.url += "lights/";
		break;
	case MqttDeviceType::MQTT_SWITCH:
		webStatus.url += "switches/";
		break;
	}

	webStatus.url += parentDevice->name.c_str();

	InitializeRequests();
	InitializePage();
}

void MqttDeviceWeb::Deinitialize()
{
	webStatus.enabled = false;

	if (webStatus.hostingStatusRequest)
	{
		server.removeHandler(handlers.statusHandler);
		server.removeHandler(handlers.stateHandler);
	}

	if (webStatus.hostingWebpage)
	server.removeHandler(handlers.pageHandler);

	if (webStatus.hostingPayloadsRequest)
		server.removeHandler(handlers.getPayloads);

	if (webStatus.hostingConfigRequest)
	{
		server.removeHandler(handlers.getConfig);
		server.removeHandler(handlers.setConfig);
	}
}

String MqttDeviceWeb::GetUrl()
{
	return webStatus.url;
}

String MqttDeviceWeb::GetUrl(const char* subFolder)
{
	String sub = webStatus.url + "/";
	sub += subFolder;
	return sub;
}

void MqttDeviceWeb::AddStatusData(JsonVariant& rootObj)
{
	JsonObject obj; 

	if (rootObj.containsKey("websiteStatus"))
		obj = rootObj["websiteStatus"].as<JsonObject>();
	else
		obj = rootObj.createNestedObject("websiteStatus");

	obj["enabled"] = webStatus.enabled;
	obj["hostingWebpage"] = webStatus.hostingWebpage;
	obj["hostingStatusRequest"] = webStatus.hostingStatusRequest;
	obj["hostingConfigRequest"] = webStatus.hostingConfigRequest;
}

void MqttDeviceWeb::InitializePage()
{
	if (!_parentDevice->deviceConfig.website.hostWebsite || webStatus.hostingWebpage) return;

	//webStatus.hostingWebpage = true;
}

void MqttDeviceWeb::InitializeRequests()
{
	if (webStatus.hostingStatusRequest) return;

	MqttDevice* device = _parentDevice;
	String tempUrl;

	//Get status for device
	{
		//Status response
		tempUrl = GetUrl("status");
		handlers.statusHandler = &server.on(tempUrl.c_str(), HTTP_GET, [device](AsyncWebServerRequest* request) {
			//this->StatusResponse(request);
			MqttDeviceWeb::GetStatusResponse(device, request);
		});

		//State response (For grabbing basic state without requesting status)
		tempUrl = GetUrl("state");
		handlers.stateHandler = &server.on(tempUrl.c_str(), HTTP_GET, [device](AsyncWebServerRequest* request) {
			//this->StatusResponse(request);
			MqttDeviceWeb::GetStateResponse(device, request);
		});

		webStatus.hostingStatusRequest = true;
	}

	//Get Config for device
	if (_parentDevice->deviceConfig.website.configurable)
	{
		tempUrl = GetUrl("config");
		handlers.statusHandler = &server.on(tempUrl.c_str(), HTTP_GET, [device](AsyncWebServerRequest* request) {
			MqttDeviceWeb::GetConfigResponse(device, request);
		});

		handlers.statusHandler = &server.on(tempUrl.c_str(), HTTP_POST, [device](AsyncWebServerRequest* request) {
			//Not implemented
			request->send(501, ContentType::textPlain, Messages::notImplemented);
			return;
			MqttDeviceWeb::SetConfigResponse(device, request);
		});

		webStatus.hostingConfigRequest = true;
	}

	//Enable or disable device.
	{
		tempUrl = GetUrl("setEnabled");
		handlers.statusHandler = &server.on(tempUrl.c_str(), HTTP_POST, [device](AsyncWebServerRequest* request) {

			if (!Network::Server::Authentication::IsAuthenticated(request))
			{
				request->send(511, ContentType::textPlain, Messages::networkAuthenticationRequried);
				return;
			}

			if (!request->hasParam("enable", true))
			{
				Label_BadRequest:
				request->send(400, ContentType::textPlain, Messages::badRequest);
				return;
			}

			String enable = request->getParam("enable", true)->value();

			DEBUG_LOG_F("%s SetEnabled(%s)", device->name.c_str(), enable.c_str());

			if (enable == "false")
			{
				device->Disable();
			}
			else if (enable == "true")
			{
				device->Enable();
			}
			else
			{
				goto Label_BadRequest;
				request->send(400, ContentType::textPlain, "Bad Request : Expecting \"enable\" : \"0\" or \"1\".");
			}

			//Respond with new state.
			MqttDeviceWeb::GetStateResponse(device, request);
			//request->send(200);

		});

		webStatus.hostingStatusRequest = true;
	}

	//Get MQTT payload strings.
	{
		tempUrl = GetUrl("payloads");
		handlers.getPayloads = &server.on(tempUrl.c_str(), HTTP_GET, [device](AsyncWebServerRequest* request) {
			MqttDeviceWeb::GetConfigResponse(device, request);
		});

		webStatus.hostingConfigRequest = true;
	}
}

#include "../../Network/Server/SpecialRequests.h"

void MqttDeviceWeb::GetStateResponse(MqttDevice* device, AsyncWebServerRequest* request)
{
	device->SetDeviceState();
	Network::Server::SpecialRequests::ResponseSerializedData("state", 128, (JsonHelper::PACK_JSON_FUNC)[device](JsonVariant& doc) {
		doc["state"].set((DeviceState)device->deviceStatus.state);
		return 0;
	}, request);
}

void MqttDeviceWeb::GetStatusResponse(MqttDevice* device, AsyncWebServerRequest* request)
{
	if (device == nullptr) return;

	String jsonData = device->GenerateJsonStatus();

	if (jsonData.isEmpty())
		request->send(204, "No JSON data was generated.");
	else
		request->send(200, Network::Website::Strings::ContentType::appJSON, jsonData);
}

void MqttDeviceWeb::GetConfigResponse(MqttDevice* device, AsyncWebServerRequest* request)
{
	if (device == nullptr) return;

		String jsonData = device->GenerateJsonConfig();

	if (jsonData.isEmpty())
		request->send(204, "No JSON data was generated.");
	else
		request->send(200, Network::Website::Strings::ContentType::appJSON, jsonData);
}

void MqttDeviceWeb::SetConfigResponse(MqttDevice* device, AsyncWebServerRequest* request)
{
	if (device == nullptr) return;

	if (!request->hasParam("config", true))
	{
		Label_BadRequest:
		DEBUG_LOG_F("%s(%s)Cannot Set Config : No POST Data\r\n", device->name.c_str(), device->DeviceName());
		request->send(400, ContentType::textPlain, "Bad Request : Requires \"config\" : (JSON DATA)");
		return;
	}

	bool save = request->hasParam("save", true);// getParam("save", true)->value() == "1";
	//bool apply = request->hasParam("apply", true);// getParam("apply", true)->value() == "1";

	String configData = request->getParam("config", true)->value();

	if (configData.isEmpty())
		goto Label_BadRequest;


	//Set config here
	if (!device->ParseConfigFromWeb(configData)) goto Label_InternalServerError;
	if (!device->Reconfigure()) goto Label_InternalServerError;


	return;

Label_InternalServerError:
	request->send(500, ContentType::textPlain, Messages::internalServerError);
}

void MqttDeviceWeb::GetPayloadsResponse(MqttDevice* device, AsyncWebServerRequest* request)
{
	if (device == nullptr) return;

	String jsonData = device->GenerateJsonConfig();

	if (jsonData.isEmpty())
		request->send(204, "No JSON data was generated.");
	else
		request->send(200, Network::Website::Strings::ContentType::appJSON, jsonData);
}

