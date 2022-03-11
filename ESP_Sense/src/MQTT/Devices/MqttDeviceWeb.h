#ifndef _MQTTDEVICEWEB_h
#define _MQTTDEVICEWEB_h

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <List.hpp>

class MqttDevice;



class MqttDeviceWeb
{
public:
	typedef struct {
		bool enabled : 1;
		bool hostingWebpage : 1;		//Hosting webpage for status and/or config
		bool hostingStatusRequest : 1;	//Handling status request.
		bool hostingConfigRequest : 1;	//Handling config requests.
		String url;
	}Status_t;

protected:

	struct {
		AsyncCallbackWebHandler* statusHandler;
		AsyncCallbackWebHandler* pageHandler;
		AsyncCallbackWebHandler* getConfig;
		AsyncCallbackWebHandler* setConfig;
	}handlers;


public:
	MqttDevice* _parentDevice;

	Status_t webStatus;

public:
	MqttDeviceWeb()
	{
		memset(&webStatus, 0, sizeof(MqttDeviceWeb::Status_t));
	}

	virtual void Initialize(MqttDevice* parentDevice);
	virtual void Deinitialize();

	virtual String GetUrl();
	virtual String GetUrl(const char* subFolder);

	virtual void AddStatusData(JsonObject& rootObj);

protected:
	virtual void InitializePage();
	virtual void InitializeRequests();

public:
	//virtual void GetStatusResponse(AsyncWebServerRequest* request);
	static void GetStatusResponse(MqttDevice* device, AsyncWebServerRequest* request);
	static void GetConfigResponse(MqttDevice* device, AsyncWebServerRequest* request);
	static void SetConfigResponse(MqttDevice* device, AsyncWebServerRequest* request);
};
#endif

