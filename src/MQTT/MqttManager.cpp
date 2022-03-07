#include "MqttManager.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include <PubSubClient.h>	
//#include <ArduinoJson.hpp>
//#include <ArduinoJson.h>


#include "../../ESPSense.h"
#include "../Config/config_master.h"
#include "../macros.h"
#include "../Config/config_mqtt.h"
#include "../Network/WifiManager.h"
#include "../GlobalDefs.h"
#include "../HelperFunctions.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);				//MQTT Client
String client_id;									//MQTT Client ID
extern DeviceStatus_t status;
extern Config_t config;
//extern String baseTopic;
extern StatusRetained_t statusRetained;
extern StatusRetainedMonitor_t statusRetainedMonitor;

extern List<FUNC_PTR> delegateDisconnect;

Mqtt::MqttMessagesSent_t mqttMessagesSent;

#if COMPILE_ACCESSPOINT
extern IPAddress* apClientIPs;
extern uint8_t apClientCount;
extern unsigned long lastClientListRefresh;
#endif



TaskHandle_t taskMqttLoop;
TaskHandle_t taskMqttPublishAvailability;
TaskHandle_t taskMqttBlink;


bool ClientConnect(bool reconnect = false);
void SelectIPAddress();
bool SetIpFromConfig(bool ap);
bool ConnectIntervalMet();


void Mqtt::Initialize()
{
	//Don't enable MQTT if Access Point mode has configOnly flag set.
	status.mqtt.enabled = status.wifi.mode != WIFI_MODE_AP || !config.wifi.accessPoint.configOnly;

	if (!status.mqtt.enabled || status.mqtt.connected) return;

#warning Should probably add configured flag to stop re-initialization. Though at the moment there is nothing that could affect it negatively.
	
	DEBUG_LOG_LN("Initializing MQTT...");

	//Set MqttCallback for receiving MQTT messages
	DEBUG_LOG_LN("...Setting MQTT Callback...");
	mqttClient.setCallback(Mqtt::Callback);

	client_id = "esp8266-client-";
	client_id += WiFi.macAddress();
	DEBUG_LOG_F("-MQTT Client Client ID = %s\r\n", client_id.c_str());

	mqttClient.setBufferSize(config.mqtt.publish.bufferSize);

	//WifiMode must be set from default.
	assert(status.wifi.mode != WIFI_MODE_NULL);

	//Set IP Mode
	status.mqtt.ipStatus.mode = status.wifi.mode;

	if (config.mqtt.broker.autoDetectIp)
	{
		DEBUG_LOG_LN("...Auto-detect IP is enabled.");
	}

	//Select IP from either EEPROM(retainedStatus), config, or autodetect,
	//and then apply the server settings.
	SelectIPAddress();
	//mqttClient.setServer(status.mqtt.ipStatus.ip, config.mqtt.broker.port);

	DEBUG_LOG_LN("...MQTT Initialized, ready to connect.");
}




bool ConnectIntervalMet()
{
	//Only attempt to connect every n seconds, or ignore if autodetect IP is true.
	if (millis() > status.mqtt.nextMqttConnectAttempt && !config.mqtt.broker.autoDetectIp)
		return true;

	return false;
}

bool SetIpFromConfig(bool ap)
{
	if (ap && !status.mqtt.ipStatus.triedConfigAP)
	{
		status.mqtt.ipStatus.triedConfigAP = true;

		if (config.mqtt.broker.ipAP[0])
		{
			status.mqtt.ipStatus.ip = config.mqtt.broker.ipAP;
			status.mqtt.ipStatus.changed = true;
			status.mqtt.ipStatus.maxRetries = config.mqtt.broker.maxRetries;

			//Only 1 IP to try, set timeout to a longer length.
			mqttClient.setSocketTimeout(config.mqtt.broker.timeout);
			return true;
		}
	}
	else if (!status.mqtt.ipStatus.triedConfigStation)
	{
		status.mqtt.ipStatus.triedConfigStation = true;

		//Exists, set IP and leave.
		if (config.mqtt.broker.ip[0])
		{
			status.mqtt.ipStatus.ip = config.mqtt.broker.ip;
			status.mqtt.ipStatus.changed = true;
			status.mqtt.ipStatus.maxRetries = config.mqtt.broker.maxRetries;

			//Only 1 IP to try, set timeout to a longer length.
			mqttClient.setSocketTimeout(config.mqtt.broker.timeout);
			return true;
		}
	}
	return false;
}

void SelectIPAddress()
{
	//Allow IP to attempt connection to the configured amount of times.
	if (status.mqtt.ipStatus.currentAttemptsCounter < status.mqtt.ipStatus.maxRetries)
	{
		status.mqtt.ipStatus.changed = false;
		return;
	}

	status.mqtt.ipStatus.currentAttemptsCounter = 0;

	bool sendMessages = !mqttMessagesSent.selectIp;

	if (sendMessages)
		mqttMessagesSent.selectIp = true;

	DEBUG_LOG_LN("...Selecting MQTT IP...");

	//Try last IP that successfully connected, if it exists. (When a connection is successful, the IP is stored in the EEPROM)
	if (!status.mqtt.ipStatus.triedRetainedIP)
	{
	RetryRetained:
		status.mqtt.ipStatus.triedRetainedIP = true;

		//Reset flags in the event that it has started over.
		status.mqtt.ipStatus.stationAutoExhausted = false;
		status.mqtt.ipStatus.accessPointAutoExhausted = false;
		status.mqtt.ipStatus.triedConfigStation = false;
		status.mqtt.ipStatus.triedConfigAP = false;

		if (statusRetained.mqtt.ip[0]) {
			DEBUG_LOG_LN("...Using IP from Retained Status...");
			status.mqtt.ipStatus.ip = statusRetained.mqtt.ip;
			status.mqtt.ipStatus.maxRetries = config.mqtt.broker.maxRetries;
			status.mqtt.ipStatus.changed = true;
			mqttClient.setSocketTimeout(config.mqtt.broker.timeout);
			goto Label_SetServer;
		}
	}

	//If APSTA mode, and config contains mqtt.wifiMode, set to config.
	if (status.wifi.mode == WIFI_MODE_APSTA)
	{
		if (config.mqtt.broker.wifiMode == WIFI_MODE_STA || config.mqtt.broker.wifiMode == WIFI_MODE_AP)
			status.mqtt.ipStatus.mode = config.mqtt.broker.wifiMode;
		else
			status.mqtt.ipStatus.mode = WIFI_MODE_AP;	//Config does not contain a specific mode, start with access point as there are less IPs to try.
	}


	//Check if all Access point IPs have been tried
	if (status.mqtt.ipStatus.mode == WIFI_MODE_AP)
		status.mqtt.ipStatus.accessPointAutoExhausted = (status.mqtt.ipStatus.ipIndex >= status.wifi.accessPoint.clientCount && config.mqtt.broker.autoDetectIp) || (status.mqtt.ipStatus.triedConfigAP && !config.mqtt.broker.autoDetectIp);

	//Check if all Station IPs have been tried (255 - 2, ignoring 0 and 1)
	if (status.mqtt.ipStatus.mode == WIFI_MODE_STA)
		status.mqtt.ipStatus.stationAutoExhausted = (/*status.mqtt.ipStatus.totalAttemptsCounter >= 0xFF - 2*/ status.mqtt.ipStatus.ipIndex == 0 && config.mqtt.broker.autoDetectIp) || (status.mqtt.ipStatus.triedConfigStation && !config.mqtt.broker.autoDetectIp);


	//If current mode is not connected, or all IPs have been exhausted, switch.
	if (status.mqtt.ipStatus.mode == WIFI_MODE_AP && (!status.wifi.accessPoint.ipAssigned || status.mqtt.ipStatus.accessPointAutoExhausted)) {

		DEBUG_LOG("...All Access Point IPs have been exhausted.");
		//status.mqtt.ipStatus.totalAttemptsCounter = 0;
		status.mqtt.ipStatus.ipIndex = 0;

		//If WIFI is connected then the WIFI mode must be in APSTATION mode.
		//Switch IP Mode to begin trying its IPs.
		if (status.wifi.station.connected && !status.mqtt.ipStatus.stationAutoExhausted)
		{
			status.mqtt.ipStatus.mode = WIFI_MODE_STA;
			DEBUG_LOG_LN("...Switching to Station IPs...");
		}
		else
		{
			status.mqtt.ipStatus.triedRetainedIP = false;
			status.mqtt.ipStatus.triedConfigAP = false;
			DEBUG_LOG_LN("...Retrying...");
			goto RetryRetained;
		}
	}
	else if (status.mqtt.ipStatus.mode == WIFI_MODE_STA && (!status.wifi.station.connected || status.mqtt.ipStatus.stationAutoExhausted)) {

		if (!mqttMessagesSent.selectIp)
			DEBUG_LOG_LN("...All WIFI Station IPs have been exhausted. Trying Access Point IPS...");
		//status.mqtt.ipStatus.totalAttemptsCounter = 0;
		status.mqtt.ipStatus.ipIndex = 0;

		//If AP has clients connected, then the WIFI mode must be in APSTATION mode,
		//Switch IP Mode to begin trying its IPs.
		if (status.wifi.accessPoint.ipAssigned && !status.mqtt.ipStatus.accessPointAutoExhausted)
		{
			status.mqtt.ipStatus.triedConfigStation = false;				//Reset flag as AP mode will try both IPs.
			status.mqtt.ipStatus.mode = WIFI_MODE_AP;
			DEBUG_LOG_LN("...Switching to Access Point IPs...");
		}
		else
		{
			status.mqtt.ipStatus.triedRetainedIP = false;
			status.mqtt.ipStatus.triedConfigStation = false;
			DEBUG_LOG_LN("...Retrying...");
			goto RetryRetained;
		}

	}

	//Reset flag, if still using it will be re-set.
	status.mqtt.ipStatus.autoDetectingStation = false;

	switch (status.mqtt.ipStatus.mode)
	{
	case WIFI_MODE_STA:
	{
		//Try IP from configuration if it exists.
		if (SetIpFromConfig(false)) break;

		//Start at a valid IP
		if (status.mqtt.ipStatus.ipIndex == 0)
		{
			status.mqtt.ipStatus.ipIndex = 2;

			//Set IP to local IP address.
			status.mqtt.ipStatus.ip[0] = 192;
			status.mqtt.ipStatus.ip[1] = 168;
			status.mqtt.ipStatus.ip[2] = 0;

			status.mqtt.ipStatus.maxRetries = config.mqtt.broker.autoMaxRetries;
			status.mqtt.ipStatus.autoDetectingStation = true;
		}

		//Set last byte to current index.
		status.mqtt.ipStatus.ip[3] = status.mqtt.ipStatus.ipIndex++;

		//Increase counter and set changed flag
		//status.mqtt.ipStatus.totalAttemptsCounter++;
		status.mqtt.ipStatus.changed = true;

		break;
	}
	case WIFI_MODE_AP:

		//Try IP from configuration if it exists.
		//AP Mode can use both IPs from the config,
		//as the secondary one may be ommitted.
		if (SetIpFromConfig(!status.mqtt.ipStatus.triedConfigAP)) break;

		//Set IP to the client IP at the current index
		status.mqtt.ipStatus.ip = apClientIPs[status.mqtt.ipStatus.ipIndex++];

		//Set changed flag
		status.mqtt.ipStatus.changed = true;
		status.mqtt.ipStatus.maxRetries = config.mqtt.broker.maxRetries;
		break;
	}

	//Make sure IP has been set.
	if (!status.mqtt.ipStatus.ip[0])
	{
		DEBUG_LOG_LN("Error : MQTT IP has not been set!");
		assert(false);
	}

Label_SetServer:
	mqttClient.setServer(status.mqtt.ipStatus.ip, config.mqtt.broker.port);
}

/// <summary>
/// Attempts to connect to an MQTT server.
/// If autoDetectIP is true, each time connect() a counter is increased, 
/// if the counter reaches max it will switch to the next IP.
/// *Connect should only be called once in the setup. If the connection is lost, use reconnect().
/// </summary>
/// <returns>True if the connection was a success, and false if otherwise.</returns>
bool Mqtt::Connect()
{
	status.mqtt.enabled = true;

	if (!status.wifi.connected)
	{
		if (!mqttMessagesSent.connectNoWifi)
		{
			DEBUG_LOG_LN("Cannot Connect to MQTT : No WIFI Connection.");
			mqttMessagesSent.connectNoWifi = true;
		}
		return false;
	}

	if (status.mqtt.connected) return true;

	if (status.config.setupComplete)
		return Reconnect();

	//Must call DisplayMessageIntervalMet before calling connect if messages are to be displayed.
	bool sendMessages = !mqttMessagesSent.connect; 
	if (sendMessages) 
		mqttMessagesSent.connect = true;

	//May be able to remove this line, as we are yielding in its calling functions.
	if (!ConnectIntervalMet() && !status.device.tasks.wifiTaskRunning) return false;

	//status.mqtt.nextMqttConnectAttempt = millis();
	if (sendMessages)
	{
		DEBUG_LOG_LN("Connecting to MQTT Server...");
		DEBUG_LOG_F("-Port : %d\r\n", config.mqtt.broker.port);
		DEBUG_LOG_F("-Username : %s\r\nPassword = %s\r\n", config.mqtt.broker.user.c_str(), config.mqtt.broker.pass.c_str());
	}

	if (sendMessages || status.mqtt.ipStatus.changed)
		DEBUG_LOG_F("-IP = %s\r\n", status.mqtt.ipStatus.ip.toString().c_str());
	else if(status.mqtt.ipStatus.currentAttemptsCounter > 0)
		DEBUG_LOG_F("-Connection Attempt : %d\r\n", status.mqtt.ipStatus.currentAttemptsCounter + 1);

	//Try to connect
	if (!ClientConnect())
	{
		//Connection failed, try next IP address.
		SelectIPAddress();

		//if (status.mqtt.ipStatus.changed)
		//	mqttClient.setServer(status.mqtt.ipStatus.ip, config.mqtt.broker.port);
	}
	else
	{
		DEBUG_LOG_LN("...MQTT Connected!");
		status.mqtt.ipStatus.autoDetectingStation = false;
	}

	return status.mqtt.connected = mqttClient.connected();
}

bool ClientConnect(bool reconnect)
{	
	bool sendMessages = !mqttMessagesSent.clientConnect; 
	if (sendMessages) mqttMessagesSent.clientConnect = true;

	//Already checked in parent functions.
	/*if (!status.wifi.connected)
	{
		if (sendMessages)
		{
			DEBUG_LOG_LN("Cannot Connect to MQTT : No WIFI Connection.");
		}
		return false;
	}*/

	status.mqtt.connectAttempts++;

	//Try to connect
	if (status.mqtt.connected = mqttClient.connect(client_id.c_str(), config.mqtt.broker.user.c_str(), config.mqtt.broker.pass.c_str()))
	{
		DEBUG_LOG_LN("...MQTT Server Connected.");

		if (statusRetainedMonitor.mqtt.ip = statusRetained.mqtt.ip != status.mqtt.ipStatus.ip)
		{
			//Save IP so auto-detect does not have to search again.
			statusRetained.mqtt.ip = status.mqtt.ipStatus.ip;
		}

		status.mqtt.nextMqttConnectAttempt = 0;

	}
	else //Connect failed
	{
		status.mqtt.ipStatus.currentAttemptsCounter++;

		if (!status.device.tasks.wifiTaskRunning)
			status.mqtt.nextMqttConnectAttempt = millis() + config.mqtt.broker.connectInterval;

		if (sendMessages)
		{
			DEBUG_LOG("...MQTT connection failed with state ");
			DEBUG_LOG_LN(mqttClient.state());
		}
	}






	Mqtt::SetConnectionLed();

	return status.mqtt.connected = mqttClient.connected();
}

bool Mqtt::Reconnect()
{
	bool sendMessages = !mqttMessagesSent.reconnect;

	//All functions that call reconnect already check for wifi connection.
	//if (!status.wifi.connected)
	//{
	//	if (sendMessages)
	//	{
	//		DEBUG_LOG_LN("Cannot Reconnect to MQTT : No WIFI Connection.");
	//		mqttMessagesSent.clientConnect = true;
	//	}
	//	return false;
	//}

	if (!status.mqtt.enabled) return false;

	//May be able to remove this line, as we are yielding in its calling functions.
	if (!ConnectIntervalMet() && !status.device.tasks.wifiTaskRunning) return false;

	if (sendMessages)
	{
		DEBUG_LOG_LN("Reconnecting to MQTT Server...");
		mqttMessagesSent.reconnect = true;
	}

	//Try to connect
	if (ClientConnect())
	{
		//Not turning off on disconnect.
		//if (!AllImportantDevicesFunctional())
		//	Tasks::StartBlinkTask();
	}

	return status.mqtt.connected = mqttClient.connected();
}

void Mqtt::Disconnect()
{
	status.mqtt.enabled = false;

	if (status.mqtt.connected)
		mqttClient.disconnect();

	status.mqtt.connected = false;

	Mqtt::SetConnectionLed();
}

void Mqtt::MonitorConnection()
{
	Mqtt::IsConnected();

	if (status.mqtt.connected)
	{
		if (!status.mqtt.enabled)
			Disconnect();
	}
	else
	{
		if (status.mqtt.enabled)
		{
			Reconnect();
			EspSense::YieldWatchdog(config.mqtt.broker.connectInterval);
		}
	}

	if(!status.mqtt.enabled && status.device.tasks.wifiTaskRunning)
		EspSense::YieldWatchdog(10000);
}

bool Mqtt::IsConnected()
{
	bool currentStatus = status.mqtt.connected;
	status.mqtt.connected = mqttClient.connected();

	if (currentStatus != status.mqtt.connected)
	{
		if (!status.mqtt.connected)
		{
			DEBUG_LOG_LN("MQTT Connection Lost!");
		}

		Mqtt::SetConnectionLed();
	}
	
	return status.mqtt.connected;
}

void Mqtt::SetConnectionLed()
{
	if (config.mqtt.ledGpio == -1) return;

	if (/*status.mqtt.connected*/mqttClient.connected())
		LedEnable(config.mqtt.ledGpio, config.mqtt.ledOn);
	else
		LedDisable(config.mqtt.ledGpio, config.mqtt.ledOn);
}


bool Mqtt::AllImportantDevicesFunctional()
{
	uint64_t* bitmap = &status.mqtt.devices.functioningDevicesImportant.bitmap0;

	DEBUG_LOG_F("Checking Important Devices State | bitmap : %000000000000000x\r\n", bitmap);

	for (uint8_t i = 0; i < (sizeof(DevicesFunctioning_t) / 8); i++)
	{
		if (*(bitmap + i) != 0)
		{
			return false;
		}
	}

	return true;
}


void Mqtt::Loop()
{
	if (!status.mqtt.enabled || !status.config.setupComplete || (!status.wifi.station.connected && !status.wifi.accessPoint.ipAssigned)) return;

	DisplayMessageIntervalMet();

	Mqtt::MonitorConnection();

	if (!status.mqtt.connected) return;

	if (!status.device.tasks.mqttTaskRunning)
		Tasks::StartMqttLoopTask();

	if (!status.device.tasks.mqttPublishAvailabilityTaskRunning)
		Tasks::StartPublishAvailabilityTask();

	if (!status.device.tasks.mqttDeviceManagerTaskRunning)
		Tasks::StartDeviceManagerLoopTask();

	//Make sure all topics are subscribed
	if (status.mqtt.devices.enabledCount != status.mqtt.devices.subscribedCount) 
		DeviceManager::SubscribeAll(); 
	else if(status.mqtt.devices.subscribedCount != 0)
		DeviceManager::UnsubscribeAll();

	if (!status.mqtt.subscribed)
		Subscribe();

	//Called in main loop
	/*EspSense::YieldWatchdog(status.mqtt.connected ? 5000 : 1);*/
}

bool Mqtt::Subscribe()
{
	DEBUG_LOG_LN("Subscribing ESP-Sense topics...");

	if (!IsConnected()) return false;

	//Set flag that all devices should be subscribed
	status.mqtt.subscribed = true;

	if (!mqttClient.subscribe(config.mqtt.topics.jsonCommand.c_str()))		//JSON command for receiving settings and other commands.
	{
		DEBUG_LOG_F("...ESP-Sense failed to subscribe.\r\n");
		return false;
	}

	return true;
}

bool Mqtt::Unsubscribe()
{	
	DEBUG_LOG_LN("Unsubscribing to ESP-Sense topics...");

	if (!IsConnected()) return false;

	status.mqtt.subscribed = false;

	if (mqttClient.unsubscribe(config.mqtt.topics.jsonCommand.c_str()))		//JSON command for receiving settings and other commands.
	{
		DEBUG_LOG_F("ESP-Sense failed to subscribe.\r\n");
		return false;
	}

	return true;
}

/// <summary>
/// Send MQTT availability message's for the device and all MQTT devices.
/// </summary>
void Mqtt::PublishAvailability()
{
	bool sendMessages = !mqttMessagesSent.publishAvailability;

	if (sendMessages)
	{
		mqttMessagesSent.publishAvailability = true;
		DEBUG_LOG_LN("Publish Availability...");
	}

	if (!IsConnected()) return;

	if (!status.device.tasks.mqttPublishAvailabilityTaskRunning)
	{
		if (millis() < status.mqtt.nextPublishAvailability + config.mqtt.publish.availabilityRate)
		{
			if (sendMessages)
				DEBUG_LOG_LN("Interval not met.");

			return;
		}
	status.mqtt.nextPublishAvailability = millis() + config.mqtt.publish.availabilityRate;
	}

	//Publish device availability.
	mqttClient.publish(config.mqtt.topics.availability.c_str(), MQTT_AVAILABLE);

	//Publish sensors availability.
	Mqtt::DeviceManager::PublishAvailability();

	//DEBUG_LOG_LN("Availability Published.");
}


/// <summary>
/// Callback for receiving MQTT messages.
/// </summary>
/// <param name="topic">The MQTT topic.</param>
/// <param name="payload">The data received for the topic.</param>
/// <param name="length">How many bytes were in the payload.</param>
void Mqtt::Callback(char* topic, byte* payload, unsigned int length)
{
	DEBUG_LOG("MQTT Message Received | Topic : ");
	DEBUG_LOG_LN(topic);
	DEBUG_LOG("Payload : ");
	DEBUG_WRITE_L(payload, length);
	DEBUG_LOG_LN();

	//Forward to devices.
	Mqtt::DeviceManager::Callback(topic, payload, length);
}


bool Mqtt::DisplayMessageIntervalMet()
{
	if (millis() > status.mqtt.nextDisplayMessages && mqttMessagesSent.bitmap != 0)
	{
		status.mqtt.nextDisplayMessages = millis() + config.mqtt.publish.errorRate;
		mqttMessagesSent.bitmap = 0;
		return true;
	}

	return false;
}



void Mqtt::Tasks::StartAllTasks()
{
	StartMqttLoopTask();
	StartPublishAvailabilityTask();
	Tasks::StartDeviceManagerLoopTask();
}

void Mqtt::Tasks::StartMqttLoopTask()
{
	if (status.device.tasks.mqttTaskRunning) return;

	status.device.tasks.mqttTaskRunning = true;


#if defined(ESP8266)
	schedule_recurrent_function_us([]() {
		bool result = Mqtt::Tasks::MqttLoopTask();

		if (!result)
		{
			status.device.tasks.mqttTaskRunning = false;
		}

		return result;
	}, config.device.taskSettings.recurRate);
#elif defined(ESP32)
	xTaskCreatePinnedToCore(
		MqttLoopTask,							// Task function
		"MQTTLoop",								// Name
		config.mqtt.taskSettings.stackSize,		// Stack size
		NULL,									// Parameters
		config.mqtt.taskSettings.priority,		// Priority
		&taskMqttLoop,							// Task handle
		config.mqtt.taskSettings.core);			// Core

#endif





}

void Mqtt::Tasks::StopLoopTask()
{
	if (!status.device.tasks.mqttTaskRunning) return;

	status.device.tasks.ftpTaskRunning = false;
	status.mqtt.enabled = false;
	vTaskDelete(taskMqttLoop);
}

#if defined(ESP8266)
bool Mqtt::MqttLoopTask()
{
	
}

#elif defined(ESP32)
void Mqtt::Tasks::MqttLoopTask(void* pvParameters) {
	DEBUG_LOG("Mqtt::Tasks::MqttLoopTask() started on core ");
	DEBUG_LOG_LN(xPortGetCoreID());

	status.device.tasks.mqttTaskRunning = true;

	unsigned long nextAliveMessage;

	for (;;) {

		//delay(1);
		EspSense::YieldWatchdog(config.mqtt.taskSettings.recurRate);

		if (!status.mqtt.connected)
			break;

		mqttClient.loop();

#if COMPILE_ALIVE_MSG
		if (millis() > nextAliveMessage)
		{
			nextAliveMessage = millis() + 10000;
			DEBUG_LOG_LN("MqttLoopTask Alive");
			EspSense::DisplayMemoryInfo();
			DEBUG_NEWLINE();
		}
#endif
	}

	DEBUG_LOG_LN("Exiting MqttLoopTask Task");

	status.device.tasks.mqttTaskRunning = false;
	vTaskDelete(NULL);
}
#endif

bool Mqtt::Tasks::StartPublishAvailabilityTask()
{
	if (!status.mqtt.devicesConfigured) return false;
	if (status.device.tasks.mqttPublishAvailabilityTaskRunning) return true;
	status.device.tasks.mqttPublishAvailabilityTaskRunning = true;

	xTaskCreatePinnedToCore(
		PublishAvailabilityTask,					// Task function
		"PublishAvailability",						// Name
		4096,										// Stack size
		NULL,										// Parameters
		2,											// Priority
		&taskMqttPublishAvailability,				// Task handle
		1);											// Core

	return true;
}

void Mqtt::Tasks::StopPublishAvailabilityTask()
{
	if (!status.device.tasks.mqttPublishAvailabilityTaskRunning) return;

	vTaskDelete(taskMqttPublishAvailability);
	status.device.tasks.mqttPublishAvailabilityTaskRunning = false;
}

void Mqtt::Tasks::PublishAvailabilityTask(void* pvParameters)
{
	status.device.tasks.mqttPublishAvailabilityTaskRunning = true;

	unsigned long nextAliveMessage;

	for (;;)
	{
		if (!status.mqtt.connected) break;

		PublishAvailability();
		EspSense::YieldWatchdog(config.mqtt.publish.availabilityRate);

#if COMPILE_ALIVE_MSG
		if (millis() > nextAliveMessage)
		{
			nextAliveMessage = millis() + 10000;
			DEBUG_LOG_LN("MqttLoopTask Alive");
			EspSense::DisplayMemoryInfo();
			DEBUG_NEWLINE();
		}
#endif
	}

	status.device.tasks.mqttPublishAvailabilityTaskRunning = false;

	vTaskDelete(NULL);
}


bool Mqtt::Tasks::StartBlinkTask()
{
	if (config.mqtt.ledGpio == -1) return false;
	if (status.device.tasks.mqttBlinkTaskRunning) return true;
	status.device.tasks.mqttBlinkTaskRunning = true;

	xTaskCreatePinnedToCore(
		BlinkTask,				// Task function
		"BlinkMqttLed",			// Name
		1024,					// Stack size
		NULL,					// Parameters
		2,						// Priority
		&taskMqttBlink,			// Task handle
		1);						// Core

	return true;
}

void Mqtt::Tasks::StopBlinkTask()
{
	if (!status.device.tasks.mqttBlinkTaskRunning) return;

	vTaskDelete(taskMqttBlink);
	status.device.tasks.mqttBlinkTaskRunning = false;
}

void Mqtt::Tasks::BlinkTask(void* pvParameters)
{
	status.device.tasks.mqttBlinkTaskRunning = true;

	for (;;)
	{
		if (status.mqtt.connected)
			LedToggle(config.mqtt.ledGpio);

		EspSense::YieldWatchdog(500);
	}

	status.device.tasks.mqttBlinkTaskRunning = false;

	vTaskDelete(NULL);
}


