#ifndef _MQTTDEVICEWEB_h
#define _MQTTDEVICEWEB_h

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <List.hpp>

#include "../../Macros.h"

class MqttDevice;



class MqttDeviceWeb
{
public:
	typedef struct {
		struct {
			bool enabled : 1;
			bool hostingWebpage : 1;		//Hosting webpage for status and/or config
			bool hostingStatusRequest : 1;	//Handling status request.
			bool hostingConfigRequest : 1;	//Handling config requests.
			bool hostingPayloadsRequest : 1;
			uint8_t reserved : 3;
		};
		String url;
	}Status_t;

protected:

	struct {
		AsyncCallbackWebHandler* statusHandler;
		AsyncCallbackWebHandler* stateHandler;
		AsyncCallbackWebHandler* pageHandler;
		AsyncCallbackWebHandler* getConfig;
		AsyncCallbackWebHandler* setConfig;
		AsyncCallbackWebHandler* getPayloads;
	}handlers;


public:
	MqttDevice* _parentDevice;

	Status_t webStatus;

public:
	MqttDeviceWeb()
	{
		DEBUG_LOG_LN("Constructing MqttDeviceWeb");
		memset(&webStatus, 0, sizeof(MqttDeviceWeb::Status_t) - sizeof(String));
		webStatus.url = "";
	}

	virtual void Initialize(MqttDevice* parentDevice);
	virtual void Deinitialize();

	virtual String GetUrl();
	virtual String GetUrl(const char* subFolder);

	virtual void AddStatusData(JsonVariant& rootObj);

protected:
	virtual void InitializePage();
	virtual void InitializeRequests();

public:
	//virtual void GetStatusResponse(AsyncWebServerRequest* request);
	static void GetStateResponse(MqttDevice* device, AsyncWebServerRequest* request);
	static void GetStatusResponse(MqttDevice* device, AsyncWebServerRequest* request);
	static void GetConfigResponse(MqttDevice* device, AsyncWebServerRequest* request);
	static void SetConfigResponse(MqttDevice* device, AsyncWebServerRequest* request);
	static void GetPayloadsResponse(MqttDevice* device, AsyncWebServerRequest* request);
};
#endif

