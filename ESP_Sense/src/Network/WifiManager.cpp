#include "WifiManager.h"

#warning Reminder to clean this file up, its a bloody mess!

#pragma region Includes

#include <PubSubClient.h>	

#if defined(ESP8266)
#include <Hash.h>
#include <HardwareSerial.h>
#include <LittleFS.h>					//File system for loading config files.
//#include <ESP8266WiFi.h>
#if COMPILE_SERVER
#include <ESP8266mDNS.h>
#include <user_interface.h>
//#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#endif
#elif defined(ESP32)
#include <esp32-hal-uart.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <FS.h>
#include <LITTLEFS.h>
#if COMPILE_SERVER
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#endif	/*COMPILE_SERVER*/
#endif	/*defined(ESP8266) else defined(ESP32)*/

#if COMPILE_SERVER
#include <ESPAsyncWebServer.h>
#endif	/*COMPILE_SERVER*/

#if COMPILE_OTA
#include <ArduinoOTA.h>					//Wifi re-flashing
#endif	/*COMPILE_OTA*/

//#include <ArduinoOTA.h>					//Wifi re-flashing
#if COMPILE_FTP
#include <FTPServer.h>
#include <FTPCommon.h>
#include "Server/FtpManager.h"
#endif	/*COMPILE_FTP*/


#include "../../ESP_Sense.h"
#include "../Config/ConfigManager.h"
#include "../MQTT/MqttManager.h"
#include "../GlobalDefs.h"
#include "../macros.h"
#include "../HelperFunctions.h"
#include "Server/ServerManager.h"
#include "Website/WebConfigBrowser.h"

#pragma endregion

#pragma region Members

//WiFiClient wifiClient;	Only used by MQTT, resides there,
#if COMPILE_SERVER
extern AsyncWebServer server;
#endif	/*COMPILE_SERVER*/


#if COMPILE_ACCESSPOINT
IPAddress* apClientIPs = nullptr;
#endif

TaskHandle_t taskWifiLoop;
TaskHandle_t taskWifiBlink;

/// <summary>
/// Keeps track of what functions have recently displayed their debug messages.
/// *Not a fan of how this is implemented, but I currently don't have a better(easier) idea.
/// </summary>
union {
	struct {
		bool connect : 1;
		bool reconnect : 1;
		bool connectInternal : 1;
		bool startAP : 1;
		bool monitor : 1;
	};
	uint8_t bitmap;
}wifiMessagesSent;

//String wifiIP;
//String accessPointIP;

#pragma endregion

#pragma region External Members

#if COMPILE_FTP
extern FTPServer ftpServer;
#endif


extern PubSubClient mqttClient;
extern Config_t config;
extern GlobalStatus_t status;

#if defined(ESP8266)
//extern void serialEventRun(void) __attribute__((weak));
#endif

#pragma endregion

#pragma region Private Function Prototypes

int InternalConnect(bool reconnect, bool waitForConnection, bool checkHotspotButton = false);

#pragma region Event Functions and IDs

void EventWifiConnected(system_event_id_t e, WiFiEventInfo_t info);
void EventWifiDisconnected(system_event_id_t e, WiFiEventInfo_t info);
void EventWifiGotIP(system_event_id_t e, WiFiEventInfo_t info);
void EventWifiLostIP(system_event_id_t e, WiFiEventInfo_t info);

wifi_event_id_t eventIdWifiConnected;
wifi_event_id_t eventIdWifiDisconnected;
wifi_event_id_t eventIdWifiGotIP;
wifi_event_id_t eventIdWifiLostIP;

#if COMPILE_ACCESSPOINT

bool GetAPClientIps(IPAddress** out_ips = nullptr);
void EventApStationConnected(system_event_id_t e, WiFiEventInfo_t info);
void EventApStationDisconnected(system_event_id_t e, WiFiEventInfo_t info);
void EventApAssignedIP(system_event_id_t e, WiFiEventInfo_t info);

wifi_event_id_t eventIdApStationConnected;
wifi_event_id_t eventIdApStationDisconnected;
wifi_event_id_t eventIdApAssignedIP; 
#endif

#pragma endregion

#pragma endregion


#pragma region Functions

#pragma region Initialization Functions

void WifiManager::Initialize()
{
	DEBUG_LOG_LN("WIFI initializing...");

	if (status.wifi.station.enabled)
	{
		DEBUG_LOG_LN("...Already initialized...");
		Deinitialize();
	}

	status.wifi.station.missingRequiredInfo = config.wifi.station.ssid.isEmpty() || config.wifi.station.pass.isEmpty();

	if (!status.wifi.station.missingRequiredInfo && !status.mqtt.missingRequiredInfo)
	{
		WifiManager::AccessPoint::CheckHotspotButton();

		if (statusRetained.boot.wifiMode != WIFI_MODE_NULL)
		{
			DEBUG_LOG_F("Using Retained Status Wifi Mode : %d\r\n", statusRetained.boot.wifiMode);
			status.wifi.mode = statusRetained.boot.wifiMode;
			//statusRetained.boot.wifiMode = WIFI_MODE_NULL;
			ChangeWifiMode(WIFI_MODE_NULL, false, false);
		}
	}

	if (status.wifi.mode == WIFI_MODE_NULL)
	{
		if (config.wifi.station.enabled && !config.wifi.accessPoint.enabled && !status.wifi.station.missingRequiredInfo)
		{
			status.wifi.mode = WIFI_MODE_STA;
		}
		else if (status.wifi.station.missingRequiredInfo || (!config.wifi.station.enabled && config.wifi.accessPoint.enabled))
		{
			status.wifi.mode = WIFI_MODE_AP;
			status.device.configMode = config.wifi.accessPoint.configOnly;
		}
		else
		{
			status.wifi.mode = WIFI_MODE_APSTA;
		}
	}


	if (statusRetained.boot.configMode != false || (status.wifi.station.missingRequiredInfo && !config.wifi.accessPoint.enabled) || status.mqtt.missingRequiredInfo)
	{
		status.device.configMode = statusRetained.boot.configMode;
		statusRetainedMonitor.boot.configMode = statusRetained.boot.configMode != false;
		statusRetained.boot.configMode = false;
	}

	//Assure debug messages are sent.
	status.wifi.nextDisplayMessage = 0;
	DisplayMessageIntervalMet();

	//Set WiFI mode

	RegisterEventHandlers();

	DEBUG_LOG_LN("...WIFI Initialized, ready to connect.");
	DEBUG_NEWLINE();
}

void WifiManager::Deinitialize()
{
	DEBUG_LOG_LN("Deinitializing WIFI...");

	if (status.wifi.station.enabled) {

		Network::Server::Deinitialize();
		Disconnect();
	}

	status.wifi.station.enabled = false;
	UnregisterEventHandlers();

	DEBUG_LOG_LN("...WIFI Deinitialized.");
	DEBUG_NEWLINE();
}

void WifiManager::RegisterEventHandlers()
{
	DEBUG_LOG_LN("...Registering WIFI Events... ");
	if (status.wifi.mode != WIFI_MODE_STA && !status.wifi.accessPoint.eventsRegistered)
	{
		eventIdApStationConnected = WiFi.onEvent(EventApStationConnected, SYSTEM_EVENT_AP_STACONNECTED);
		eventIdApAssignedIP = WiFi.onEvent(EventApAssignedIP, SYSTEM_EVENT_AP_STAIPASSIGNED);
		eventIdApStationDisconnected = WiFi.onEvent(EventApStationDisconnected, SYSTEM_EVENT_AP_STADISCONNECTED);
		status.wifi.accessPoint.eventsRegistered = true;
	}

	if (status.wifi.mode != WIFI_MODE_AP && !status.wifi.station.eventsRegistered)
	{
		eventIdWifiConnected = WiFi.onEvent(EventWifiConnected, SYSTEM_EVENT_STA_CONNECTED);
		eventIdWifiGotIP = WiFi.onEvent(EventWifiGotIP, SYSTEM_EVENT_STA_GOT_IP);
		eventIdWifiDisconnected = WiFi.onEvent(EventWifiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
		eventIdWifiLostIP = WiFi.onEvent(EventWifiLostIP, SYSTEM_EVENT_STA_LOST_IP);
		status.wifi.station.eventsRegistered = true;
	}

	status.wifi.eventsRegistered = true;
}

void WifiManager::UnregisterEventHandlers()
{
	//if (!status.wifi.eventsRegistered) return;

	DEBUG_LOG_LN("...Unregistering WIFI Events... ");

	if (status.wifi.station.eventsRegistered)
	{
		WiFi.removeEvent(eventIdWifiConnected);
		WiFi.removeEvent(eventIdWifiGotIP);
		WiFi.removeEvent(eventIdWifiDisconnected);
		WiFi.removeEvent(eventIdWifiLostIP);
		status.wifi.station.eventsRegistered = false;
	}

	if (status.wifi.accessPoint.eventsRegistered)
	{
		WiFi.removeEvent(eventIdApStationConnected);
		WiFi.removeEvent(eventIdApAssignedIP);
		WiFi.removeEvent(eventIdApStationDisconnected);
		status.wifi.accessPoint.eventsRegistered = false;
	}	

	status.wifi.eventsRegistered = false;
}

bool WifiManager::SetPowerLevel(wifi_power_t power)
{
	DEBUG_LOG_LN("...Setting WIFI TX Power...");
	WiFi.setTxPower(power);
}

wifi_power_t WifiManager::GetPowerLevel()
{
	wifi_power_t power = WiFi.getTxPower();
}


void WifiManager::Loop()
{
	//Testing to see if FTP is more stable.
	if (status.device.tasks.ftpTaskRunning /*|| status.device.tasks.otaTaskRunning*/)
	{
		WifiManager::DisplayMessageIntervalMet();
		EspSense::YieldWatchdog(50);
		WifiManager::MonitorConnection();
		EspSense::YieldWatchdog(50);
		//AccessPoint::CheckHotspotButton();
		//EspSense::YieldWatchdog(50);
	}
	else
	{
		WifiManager::DisplayMessageIntervalMet();
		WifiManager::MonitorConnection();
		//AccessPoint::CheckHotspotButton();
		EspSense::YieldWatchdog(50);
	}


	if (!status.wifi.connected) return;

	////Only run MQTT on main thread.
	//if (status.config.setupComplete && !status.device.tasks.wifiTaskRunning)
	//{
	//	Mqtt::Loop();
	//}


	Network::Server::Loop();	

	if (status.device.tasks.ftpTaskRunning /*|| status.device.tasks.otaTaskRunning*/)
	EspSense::YieldWatchdog(250);

	#warning ignoring access pointand dual core.

		//#if COMPILE_ACCESSPOINT
		//	AccessPoint::CheckHotspotButton();
		//	if (!status.wifi.station.connected && !status.wifi.accessPoint.connected) return;
		//#else
		//	if (!status.wifi.station.connected) return;
		//#endif
		//
		//	EspSense::YieldWatchdog(1);
		//
		//#if COMPILE_TASKS
		//	#warning temporary
		//		if (!status.device.tasks.wifiTaskRunning && !config.device.taskSettings.enabled)
		//			Mqtt::Loop();
		//#else
		//	Mqtt::Loop();	//Only run Mqtt Loop on main core.
		//#endif
		//
		//	EspSense::YieldWatchdog(1);
		//
		//#if COMPILE_SERVER
		//	Network::Server::Loop();
		//#elif COMPILE_FTP
		//	Network::FTP::Loop();
		//#endif
		//
		//	EspSense::YieldWatchdog(5);
}

void WifiManager::ChangeWifiMode(wifi_mode_t mode, bool restart, bool saveRetained)
{

	if (mode < 1 || mode > 3) return;	//Invalid

	#warning never used
		DEBUG_LOG_LN("Changing Wifi Mode for next boot only...");

	statusRetainedMonitor.boot.wifiMode = statusRetained.boot.wifiMode != mode;
	statusRetained.boot.wifiMode = mode;

	if (saveRetained || restart)
		Config::Status::SaveRetainedStatus();

	if (restart)
	{
		DEBUG_LOG_LN("...Restarting Device.");
		EspSense::RestartDevice();
	}
}


#pragma endregion

#pragma region Connection Functions

int WifiManager::Connect()
{

#if COMPILE_ACCESSPOINT
#if defined(ESP8266)
#error Required
#elif defined (ESP32)

	switch (status.wifi.mode)
	{
	case WIFI_MODE_STA:
		DEBUG_LOG_LN("...Station Mode...");
		WiFi.mode(WIFI_STA);
		WifiManager::ConnectWifi(true);
		break;
	case WIFI_MODE_AP:
		DEBUG_LOG_LN("...Access Point Mode...");
		WiFi.mode(WIFI_AP);		
		AccessPoint::StartAdvertising(true);
		break;
	case WIFI_MODE_APSTA:
		DEBUG_LOG_LN("...Accesspoint/Station Mode...");
		WiFi.mode(WIFI_AP_STA);
		AccessPoint::StartAdvertising(false);
		status.wifi.nextDisplayMessage = 0;
		WifiManager::ConnectWifi(false);

		//Blink LEDS to show that they are enabled and waiting for a connection.
		Tasks::StartBlinkTask(true, true, 1000);

		EspSense::YieldWatchdog(5000);

		if (!status.wifi.connected /*WiFi.status() != WL_CONNECTED || !status.wifi.accessPoint.ipAssigned*/)
		{
			DEBUG_LOG_LN("...Waiting to connect or for a client to connect to the Access Point...");

			//If WiFi hasn't already connected, it may not.
			//Usually retained status is saved at end of setup, but it may not get there so save now.
			if (statusRetainedMonitor.boot.bitmap != 0 && !status.config.saveRetainedLoop)
				Config::Status::SaveRetainedStatus();
		}


		while (!status.wifi.connected /*(WiFi.status() != WL_CONNECTED) && !status.wifi.accessPoint.ipAssigned*/)
		{
			EspSense::YieldWatchdog(500);

			wl_status_t currentStatus = WiFi.status();

			if (CanAttemptConnection())
				WifiManager::ConnectWifi(false);
		}
		break;
	}

	Tasks::StartBlinkTask(status.wifi.station.connected, status.wifi.accessPoint.ipAssigned, 500);

#endif
#else
	WiFi.mode(WIFI_STA);
	WifiManager::ConnectWifi(false);
#endif

}

int WifiManager::ConnectWifi(bool waitForConnection)
{
	status.wifi.station.enabled = true;

	if (DisplayMessageIntervalMet() || !wifiMessagesSent.connect)
	{
		DEBUG_LOG_LN("Connecting to Wifi...");
		wifiMessagesSent.connect = true;
	}
	#warning spacing	
	int errCode = InternalConnect(false, waitForConnection, true);
	
	//if (status.wifi.mode == WIFI_MODE_APSTATION)
	//{
	//	//Give time to internal tasks, and allow events to fire,
	//	//as in APSTA mode it does not wait in a loop.
	//	delay(5000);
	//	yield();
	//}

	return errCode;
}

int InternalConnect(bool reconnect, bool waitForConnection, bool checkHotspotButton)
{

	bool canDisplayMessages = !wifiMessagesSent.connectInternal;

	//Mark that messages have been sent.
	if (canDisplayMessages)
		wifiMessagesSent.connectInternal = true;

	if (status.wifi.mode == WIFI_MODE_AP)
	{
		if (canDisplayMessages)
		{
			DEBUG_LOG_LN("...Incorrect mode.");
			DEBUG_NEWLINE();
		}
		return 3;	//Incorrect mode.
	}

	if (WiFi.status() == WL_CONNECTED)
	{
		status.wifi.station.connected = true;
		if (canDisplayMessages)
		{
			DEBUG_LOG_LN("...Already connected.");
			DEBUG_NEWLINE();
		}
		return 0;	//Already connected.
	}


//	if (checkHotspotButton)
//		if (WifiManager::AccessPoint::CheckHotspotButton())
//		{
//			return 2;
//		}

	if (canDisplayMessages)
	{
		DEBUG_LOG_LN("...Attempting to connect...");
		DEBUG_LOG_F("SSID : %s\r\nPass : %s\r\n", config.wifi.station.ssid.c_str(), config.wifi.station.pass.c_str());
		DEBUG_LOG_LN("Warning : COM port may not work properly if device does not have enough power!");
	}

	Label_RetryConnect:
	if (reconnect)
		WiFi.reconnect();
	else
		WiFi.begin(config.wifi.station.ssid.c_str(), config.wifi.station.pass.c_str(), config.wifi.channel);

	
	status.wifi.connectAttempts++;
	status.wifi.station.attemptTimestamp = millis();
	
	//status.wifi.station.startedConnecting = true;

	//If Station mode only, wait for connection success.
	if (status.wifi.mode == WIFI_MODE_STA)
	{
		WifiManager::Tasks::StartBlinkTask(true, false, 1000);
		EspSense::YieldWatchdog(5000);

		//If WiFi hasn't already connected, it may not.
		//Usually retained status is saved at end of setup, but it may not get there so save now.
		if (WiFi.status() != WL_CONNECTED && statusRetainedMonitor.boot.bitmap != 0 && !status.config.saveRetainedLoop)
			Config::Status::SaveRetainedStatus();
	}

		while (waitForConnection && WiFi.status() != WL_CONNECTED)
		{
			EspSense::YieldWatchdog(500);

			wl_status_t currentStatus = WiFi.status();

			if(WifiManager::CanAttemptConnection())
				goto Label_RetryConnect;

			//if (checkHotspotButton)
			//	if (WifiManager::AccessPoint::CheckHotspotButton())
			//	{
			//		return 2;
			//	}
		}
	//}
	//else 
	//{
	//	//Give time to internal tasks and for events to fire.
	//	EspSense::YieldWatchdog(1000);
	//}

	if (!status.wifi.station.connected)
	{
		if (canDisplayMessages)
		{
			if (status.wifi.mode != WIFI_MODE_APSTA)
				DEBUG_LOG_LN("...Connection failed.");

			wifiMessagesSent.connectInternal = true;
		}
	}

	//WifiManager::SetWifiConnectionLed();
	DEBUG_NEWLINE();
	return !status.wifi.station.connected;
}

int WifiManager::Reconnect(bool waitForConnection)
{
	if (!status.wifi.station.enabled) return 4;

	if (!CanAttemptConnection()) return 5;

	if (DisplayMessageIntervalMet() && !wifiMessagesSent.connect)
	{
		DEBUG_LOG_LN("Reconnecting to Wifi...");
		wifiMessagesSent.reconnect = true;
	}

	int result = InternalConnect(true, waitForConnection, false);

	if (result == 0 && !status.wifi.accessPoint.enabled)
	{
		//Allow time for other tasks to finish and connection to stabilize.
		EspSense::YieldWatchdog(5000);
	}

	return result;
}

void DisableNetworkComponents()
{
#if COMPILE_ACCESSPOINT
	if (status.wifi.station.enabled || status.wifi.accessPoint.enabled) return;	//Keep alive as long as one WiFi mode is enabled.
#endif

#if CPU_DUALCORE
	status.device.tasks.enabled = false;
	//EspSense::DisableWatchdog();

	//Wait for Loop Task to exit.
	while (status.device.tasks.wifiTaskRunning)
	{
		//EspSense::FeedWatchdog();
		delay(100);
	}
	//EspSense::EnableWatchdog();
#endif

	Mqtt::Disconnect();
#if COMPILE_SERVER || COMPILE_FTP
	Network::Server::StopServer();
#elif COMPILE_FTP
	Network::FTP::Stop();
#endif
}

uint8_t WifiManager::Disconnect()
{
	uint8_t returnCode = 0;

	if (status.wifi.mode != WIFI_MODE_AP)
	{
		if (!DisconnectWifi())
			returnCode |= 0x01;
	}

	if (status.wifi.mode != WIFI_MODE_STA)
	{
		if (!AccessPoint::Disconnect())
			returnCode |= 0x02;
	}

	return returnCode;
}

bool WifiManager::DisconnectWifi()
{
	//Set status to disabled so Disconnection event produces correct message.
	status.wifi.station.enabled = false;

	DisableNetworkComponents();

	status.wifi.station.connected = WiFi.disconnect();

	//SetWifiConnectionLed();

	return status.wifi.station.connected;
}

bool WifiManager::CanAttemptConnection()
{
	wl_status_t wifiStatus = WiFi.status();
	//if (!status.wifi.station.startedConnecting)
	return status.wifi.station.enabled && !status.wifi.station.connected && (millis() > status.wifi.station.attemptTimestamp || wifiStatus == WL_CONNECTION_LOST || wifiStatus == WL_CONNECT_FAILED || wifiStatus == WL_NO_SHIELD);
}

void WifiManager::MonitorConnection()
{
	if (status.wifi.station.connected)
	{
		if (!status.wifi.station.enabled)
			DisconnectWifi();	
}
	else
	{
		if (status.wifi.station.enabled && !status.wifi.station.missingRequiredInfo)
		{
			Reconnect(!status.wifi.accessPoint.enabled);
		}
	}

#if COMPILE_ACCESSPOINT

#warning temporary backup, events should cover this.
	if (WifiManager::AccessPoint::GetApClientCount() == 0 && status.wifi.accessPoint.ipAssigned)
	{
		status.wifi.accessPoint.ipAssigned = false;
	}

	if (status.wifi.accessPoint.connected)
	{
		if (!status.wifi.accessPoint.enabled)
			Disconnect();
	}
	else
	{
		if (status.wifi.accessPoint.enabled)
			if (AccessPoint::StartAdvertising())
			{				
				if (!status.wifi.station.enabled)
				{
					//Allow time for other tasks to finish and connection to stabilize.
					EspSense::YieldWatchdog(5000);
				}
			}
	}
#endif

	if (!status.device.tasks.wifiTaskRunning && status.device.tasks.enabled)
		Tasks::StartWifiLoopTask();

}

void WifiManager::SetWifiConnectionLed()
{
	if (config.wifi.station.ledGpio == -1) return;
	
	if(status.wifi.station.connected)
		LedEnable(config.wifi.station.ledGpio, config.wifi.station.ledOn);
	else
		LedDisable(config.wifi.station.ledGpio, config.wifi.station.ledOn);
}

//const char* WifiManager::GetIPString()
//{
//	return wifiIP.c_str();
//}

#pragma endregion


bool WifiManager::DisplayMessageIntervalMet()
{
	if (millis() > status.wifi.nextDisplayMessage)
	{
		status.wifi.nextDisplayMessage = millis() + (WIFI_MESSAGE_DISPLAY_RATE * 1000);

		wifiMessagesSent.bitmap = 0;
		return true;
	}

	return false;
}


#pragma region Events

#if COMPILE_ACCESSPOINT
void EventWifiConnected(system_event_id_t e, WiFiEventInfo_t info)
{
	DEBUG_LOG_LN("Wifi Successfuly Connected!");
	status.wifi.station.connected = true;
	status.wifi.connected = true;
	//status.wifi.station.startedConnecting = false;
	//WifiManager::SetWifiConnectionLed();

	WifiManager::Tasks::StartBlinkTask(true, status.wifi.accessPoint.blink, 250, 5000);
}

void EventWifiDisconnected(system_event_id_t e, WiFiEventInfo_t info)
{
	//Set connected to false instantly so tasks know ASAP.
	status.wifi.station.connected = false;
	//status.wifi.station.startedConnecting = false;
	status.wifi.connected = status.wifi.accessPoint.ipAssigned;
	WifiManager::SetWifiConnectionLed();

	if (status.wifi.station.enabled)
	{
		DEBUG_LOG_LN("Wifi Lost Connection!");
	}
	else
	{
		DEBUG_LOG_LN("WIFI Disconnected.");
	}
}

void EventWifiGotIP(system_event_id_t e, WiFiEventInfo_t info)
{
	DEBUG_LOG_LN("WIFI was assigned an IP.");

	if (info.got_ip.ip_changed)
	{
		//Not sure if this is set the first time it is assigned.

		DEBUG_LOG_LN("IP Changed.");
	}

	status.wifi.station.gotIP = true;

	status.wifi.station.ip = IPAddress(info.got_ip.ip_info.ip.addr);
	//wifiIP = status.wifi.station.ip.toString();

	DEBUG_LOG("IP: ");
	DEBUG_LOG_LN(ip4addr_ntoa(&(info.got_ip.ip_info.ip)));
}

void EventWifiLostIP(system_event_id_t e, WiFiEventInfo_t info)
{
	DEBUG_LOG_LN("WIFI has lost its IP.");
	status.wifi.station.gotIP = false;

	status.wifi.station.ip = IPAddress();
	//wifiIP = "0.0.0.0";
}

#pragma region Hotspot Events

void EventApStationConnected(system_event_id_t e, WiFiEventInfo_t info)
{
	DEBUG_LOG_LN("Client connected to Access Point.");

#if SHOW_DEBUG_MESSAGES	
	for (int i = 0; i < 6; i++) {

		DEBUG_LOG_F("%02X", info.sta_connected.mac[i]);
		if (i < 5)DEBUG_LOG(":");
	}
	DEBUG_NEWLINE();
#endif

	////Refresh client IPs
	//GetAPClientIps();			/*Called from EventApAssignedIP() */
}

void EventApStationDisconnected(system_event_id_t e, WiFiEventInfo_t info)
{
	DEBUG_LOG_LN("Client disconnected from Access Point.");
	//Refresh client IPs
	GetAPClientIps();

	if (status.wifi.accessPoint.clientCount == 0)
		status.wifi.accessPoint.ipAssigned = false;

	status.wifi.connected = status.wifi.station.connected;

	WifiManager::AccessPoint::SetAccessPointConnectionLed();
}

void EventApAssignedIP(system_event_id_t e, WiFiEventInfo_t info)
{
	DEBUG_LOG_LN("IP Assigned...");

	#warning need to make sure event is called when device has configured a static IP

	//Allow 2 seconds for connection to stabilizie. 
	//Event can be called many times first time connecting,
	//as it may not keep the first IP.
	EspSense::YieldWatchdog(status.wifi.mode == WIFI_MODE_AP ? 5000 : 2000);

	GetAPClientIps();

	if (status.wifi.mode == WIFI_MODE_AP)
		delay(2000);

	status.wifi.connected = status.wifi.accessPoint.ipAssigned;

	//WifiManager::AccessPoint::SetAccessPointConnectionLed();
	WifiManager::Tasks::StartBlinkTask(status.wifi.station.blink, true, 250, 5000);
}
#endif // COMPILE_ACCESSPOINT

#pragma endregion

#pragma endregion


#pragma region Access Point
#if COMPILE_ACCESSPOINT

bool WifiManager::AccessPoint::StartAdvertising(bool waitForConnection)
{

	bool canDisplayMessages = DisplayMessageIntervalMet() || !wifiMessagesSent.startAP;

	//Mark that messages have been sent.
	if (canDisplayMessages)
		wifiMessagesSent.startAP = true;

	if (config.wifi.accessPoint.ledGpio != -1)
		digitalWrite(config.wifi.accessPoint.ledGpio, HIGH);

	if (canDisplayMessages)
	{
		DEBUG_LOG_LN("Advertising Access Point...");
		DEBUG_LOG_F("SSID : %s\r\nPASS : %s\r\n", status.device.configMode ? WIFI_ACCESSPOINT_SSID : config.wifi.accessPoint.ssid.c_str(), status.device.configMode ? WIFI_ACCESSPOINT_PASS : config.wifi.accessPoint.pass.c_str());
	}

	delay(1000);

	status.wifi.accessPoint.enabled = true;

	if (status.device.configMode)
		status.wifi.accessPoint.connected = WiFi.softAP(WIFI_ACCESSPOINT_SSID, WIFI_ACCESSPOINT_PASS);
	else
		status.wifi.accessPoint.connected = WiFi.softAP(config.wifi.accessPoint.ssid.c_str(), config.wifi.accessPoint.pass.c_str(), config.wifi.channel, config.wifi.accessPoint.hidden, config.wifi.accessPoint.maxConnections);

	#warning WIP. if AP mode is not started.

		if (!status.wifi.accessPoint.connected)
		{
			if (canDisplayMessages)
			{
				DEBUG_LOG_LN("Failed to start Access Point Mode...");
			}

			return false;
		}

	
	status.wifi.accessPoint.ip = WiFi.softAPIP();
	//accessPointIP = status.wifi.accessPoint.ip.toString();

	if (status.wifi.mode == WIFI_MODE_AP)
		Tasks::StartBlinkTask(false, true, 1000);

	//if (status.wifi.mode == WIFI_MODE_AP)
	//{
		unsigned long canSaveRetained = millis() + FORCE_SAVE_RETAINED_AFTER * 1000;

		//Wait for a device to connect.
		DEBUG_LOG_LN("...Wait for device to connect...");

		while (waitForConnection && !status.wifi.accessPoint.ipAssigned)
		{
			//If client hasn't connected, it may not.
			//Usually retained status is saved at end of setup, but it may not get there so save now.
			if (!status.wifi.accessPoint.ipAssigned && millis() > canSaveRetained && statusRetainedMonitor.boot.bitmap != 0 && !status.config.saveRetainedLoop)
			{
				canSaveRetained = 0xFFFFFFFFFFFFFFFF;
				Config::Status::SaveRetainedStatus();
			}

			EspSense::YieldWatchdog(1000);
		}

		DEBUG_LOG_LN("...Device connecting...");	//station num is set before PC actually finishes connecting
		EspSense::YieldWatchdog(10000);
		DEBUG_LOG_LN("...Device connected...");	//#warning Need to make sure device is actually connected

	//}
	//else
	//{
	//	EspSense::YieldWatchdog(2000);
	//}



	DEBUG_LOG_F("IP : %s\r\n", status.wifi.accessPoint.ip.toString().c_str());


	if (config.server.dns)
	{
		DEBUG_LOG_F("URL : %s.local:(PORT)\r\n", config.server.hostname.c_str());
	}

	return true;
}

bool WifiManager::AccessPoint::Disconnect()
{
	status.wifi.accessPoint.enabled = false;

	DisableNetworkComponents();

	if ((status.wifi.accessPoint.connected = WiFi.softAPdisconnect()) == false)
	{
		DEBUG_LOG_LN("...Access Point Disabled.");
		status.wifi.accessPoint.clientCount = 0;
	}

	//SetAccessPointConnectionLed(); *Handled in event.

	return status.wifi.accessPoint.connected;
}

uint8_t WifiManager::AccessPoint::GetApClientCount()
{
	//if (status.wifi.mode == WIFI_MODE_STA)
	//	return status.wifi.accessPoint.clientCount = 0;

	return status.wifi.accessPoint.clientCount = WiFi.softAPgetStationNum();
}

/// <summary>
/// Change WiFi mode during runtime.
/// </summary>
void RuntimeChangeMode()
{
#warning unfinished
//Need to stop tasks. or set status to disabled and wait for them to stop.


	//DEBUG_LOG_LN("...Disconnecting Network...");
	//WifiManager::Deinitialize();

	//status.server.configured = false;

	//delay(2000);

	//if (status.wifi.mode == WIFI_MODE_APSTA)
	//{
	//	DEBUG_LOG_LN("...Enabling Config Only Mode.");
	//	DEBUG_NEWLINE();
	//	status.device.configMode = true;
	//	return false;
	//}
	//else if (status.wifi.mode == WIFI_MODE_STA)
	//{
	//	#warning Untested
	//	//Change to AP mode.
	//	DEBUG_LOG_LN("...Switching from Station to Access Point...");
	//	ChangeWifiMode(WIFI_MODE_AP, true, true);

	//	DEBUG_LOG_LN("...Please reset the device.");

	//	while (true)
	//	{
	//		EspSense::YieldWatchdog(500);

	//		if (config.wifi.station.ledGpio != -1)
	//			LedToggle(config.wifi.station.ledGpio);

	//		if (config.wifi.accessPoint.ledGpio != -1)
	//			LedToggle(config.wifi.accessPoint.ledGpio);

	//	}

	//	status.wifi.mode = WIFI_MODE_AP;
	//	status.wifi.station.enabled = false;
	//	status.wifi.accessPoint.enabled = false;
	//	status.device.configMode = true;
	//	//StartAdvertising();
	//}
	//else if (status.wifi.mode == WIFI_MODE_AP)
	//{
	//	#warning Untested
	//	//Change to station mode.
	//	//Will try to reconnect in wifi loop.
	//	//No need to do anything here.
	//	DEBUG_LOG_LN("...Enabling Config Only Mode.");


	//}

	//DEBUG_NEWLINE();
	//WifiManager::Initialize();


	//return true;
}

/// <summary>
/// Change mode during setup, before WiFi initialization.
/// </summary>
void SetupChangeMode()
{
	if (status.wifi.mode != WIFI_MODE_AP)
	{
		DEBUG_LOG_LN("...Forcing Access Point Mode...");
		status.wifi.mode = WIFI_MODE_APSTA;
	}

	DEBUG_LOG_LN("...Enabling Config Only Mode.");
	DEBUG_NEWLINE();

	status.device.configMode = true;
}

bool WifiManager::AccessPoint::CheckHotspotButton()
{
#warning currently only allowing hotspot button at setup, before wifi is initialized.
	if (status.config.setupComplete || status.device.tasks.wifiTaskRunning) return false;


	//Button not supported.
	if (config.wifi.accessPoint.buttonGpio == -1) return false;

	//DEBUG_LOG_LN("Checking Hotspot Button...");

	bool buttonPressed = false;

	
	if (ButtonGetState(config.wifi.accessPoint.buttonGpio, config.wifi.accessPoint.buttonPress))
	{
		DEBUG_LOG_F("Hotspot Button Pressed...\r\n...Delaying for holdTime(%dsec)....", config.wifi.accessPoint.holdTime / 1000);
		Tasks::StartBlinkTask(status.wifi.station.blink, true, 2000, config.wifi.accessPoint.holdTime);
		delay(config.wifi.accessPoint.holdTime);


		//Held for n seconds. Enter accessPoint mode.
		if (ButtonGetState(config.wifi.accessPoint.buttonGpio, config.wifi.accessPoint.buttonPress))
		{
			DEBUG_LOG_LN("...Hotspot Button Held...");
			buttonPressed = true;
		}
	}

	if (!buttonPressed)	return false;

	//Blink LEDs
	if(config.wifi.station.ledGpio != -1 || config.wifi.accessPoint.ledGpio != -1 || config.mqtt.ledGpio != -1)
		for (uint8_t i = 0; i < 10; i++)
		{
			EspSense::YieldWatchdog(100);

			if (config.wifi.station.ledGpio != -1)
				LedToggle(config.wifi.station.ledGpio);

			if (config.wifi.accessPoint.ledGpio != -1)
				LedToggle(config.wifi.accessPoint.ledGpio);

			if (config.mqtt.ledGpio != -1)
				LedToggle(config.mqtt.ledGpio);
		}

	if (status.config.setupComplete)
		RuntimeChangeMode();
	else
		SetupChangeMode();

	return true;
}




void WifiManager::AccessPoint::SetAccessPointConnectionLed()
{
	if (config.wifi.accessPoint.ledGpio == -1) return;

	if (status.wifi.accessPoint.ipAssigned)
		LedEnable(config.wifi.accessPoint.ledGpio, config.wifi.accessPoint.ledOn);
	else
		LedDisable(config.wifi.accessPoint.ledGpio, config.wifi.accessPoint.ledOn);
}

//const char* WifiManager::AccessPoint::GetIPString()
//{
//	return accessPointIP.c_str();
//}

bool GetAPClientIps(IPAddress** out_ips)
{
	DEBUG_LOG_LN("Get Access Point IPs...");

	WifiManager::AccessPoint::GetApClientCount();

	if (status.wifi.accessPoint.clientCount == 0)
	{
		DEBUG_LOG_LN("...No stations connected.");
		return false;
	}

#if defined(ESP8266)
	struct station_info* stat_info;
	stat_info = wifi_softap_get_station_info();

	for (uint8_t i = 0; i < stationCount; i++)
	{
		DEBUG_LOG("Station #");
		DEBUG_LOG_LN(i);

		clientIPs[i] = IPAddress(stat_info[i].ip.addr);
		DEBUG_LOG("\r\nIP: ");
		DEBUG_LOG_LN(ip4addr_ntoa(&(stat_info[i].ip)));
	}

#elif defined(ESP32)
	wifi_sta_list_t wifi_sta_list;
	tcpip_adapter_sta_list_t adapter_sta_list;

	memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
	memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

	esp_wifi_ap_get_sta_list(&wifi_sta_list);
	tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

	if (adapter_sta_list.num <= 0 || adapter_sta_list.num > 10)
	{
		DEBUG_LOG_LN("...Getting station info failed.");
		return false;
	}

	status.wifi.accessPoint.ipAssigned = true;

	uint8_t stationCount = (uint8_t)adapter_sta_list.num;

	if (apClientIPs == nullptr)
	{
		apClientIPs = (IPAddress*)malloc(sizeof(IPAddress) * stationCount);
	}
	else
	{
		if (stationCount > status.wifi.accessPoint.clientCount)
			apClientIPs = (IPAddress*)realloc(apClientIPs, sizeof(IPAddress) * stationCount);
	}

	for (uint8_t i = 0; i < stationCount; i++) {

		tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];

		DEBUG_LOG("Station #");
		DEBUG_LOG_LN(i);

		DEBUG_LOG("MAC : ");

		for (int i = 0; i < 6; i++) {

			DEBUG_LOG_F("%02X", station.mac[i]);
			if (i < 5)DEBUG_LOG(":");
		}

		IPAddress ip = IPAddress(station.ip.addr);

		apClientIPs[i] = ip;

		DEBUG_LOG("\r\nIP: ");
		DEBUG_LOG_LN(ip4addr_ntoa(&(station.ip)));
	}
#endif

	if (out_ips != nullptr)
		*out_ips = apClientIPs;

	DEBUG_LOG_LN("...Client IPs retrieved and stored.");

	DEBUG_NEWLINE();

	return true;
}


#endif
#pragma endregion

#pragma endregion

#pragma region Tasks

void WifiManager::Tasks::StartWifiLoopTask()
{
	if (status.device.tasks.wifiTaskRunning) return;
	status.device.tasks.wifiTaskRunning = true;

#if defined(ESP8266)
	schedule_recurrent_function_us([]() {
		bool result = WifiManager::LoopTask();

		if (!result)
		{
			status.device.tasks.wifiTaskRunning = false;
		}

		return result;
	}, config.device.taskSettings.recurRate);
#elif defined(ESP32)
	xTaskCreatePinnedToCore(
		Tasks::WifiLoopTask,					// Task function
		"LoopTask",								// Name
		config.wifi.taskSettings.stackSize,		// Stack size
		NULL,									// Parameters
		config.wifi.taskSettings.priority,		// Priority
		&taskWifiLoop,							// Task handle
		config.wifi.taskSettings.core);			// Core
#endif
}

void WifiManager::Tasks::StopWifiLoopTask()
{
	if (!status.device.tasks.wifiTaskRunning) return;

	status.device.tasks.wifiTaskRunning = false;
	vTaskDelete(taskWifiLoop);
}


#if defined(ESP8266)
bool WifiManager::Tasks::WifiLoopTask()
{
	status.device.tasks.taskRunning = true;

#if COMPILE_ACCESSPOINT

	if (!status.device.tasks.enabled ||
		(status.wifi.mode == WIFI_MODE_STATION && !status.wifi.station.enabled) ||
		(status.wifi.mode == WIFI_MODE_ACCESSPOINT && !status.wifi.accessPoint.enabled) ||
		(status.wifi.mode == WIFI_MODE_APSTATION && !status.wifi.accessPoint.enabled && !status.wifi.station.enabled))
		return false;
#else
	//If disabled break so task can finish.
	if (!status.device.tasks.enabled || (!status.wifi.station.enabled)) return false;
#endif

#if SHOW_DEBUG_MESSAGES
	if (millis() > core1LoopAliveMessageTimestamp + 10000)
	{
		Serial.println("Core1Loop");
		DEBUG_LOG_LN("Core1Loop1");
		core1LoopAliveMessageTimestamp = millis();
	}
#endif

	WifiManager::Loop();

	return true;
}

#elif defined(ESP32)
void WifiManager::Tasks::WifiLoopTask(void* pvParameters) {
	DEBUG_LOG("TaskManager::LoopTask() started on core ");
	DEBUG_LOG_LN(xPortGetCoreID());

	status.device.tasks.wifiTaskRunning = true;

	unsigned long nextAliveMessage;

	for (;;) {

		//delay(1);
		//EspSense::YieldWatchdog(1);

#if COMPILE_ALIVE_MSG
		if (millis() > nextAliveMessage)
		{
			nextAliveMessage = millis() + 10000;
			DEBUG_LOG_LN("WifiLoopTask Alive");
			EspSense::DisplayMemoryInfo();
			DEBUG_NEWLINE();
		}
#endif

		//if (!status.device.tasks.enabled || (!status.wifi.station.enabled && !status.wifi.accessPoint.enabled))
		//	break;

		WifiManager::Loop();

		EspSense::YieldWatchdog(status.wifi.connected ? 2000 : 1);
	}

	DEBUG_LOG_LN("Exiting WifiLoopTask Task");

	status.device.tasks.wifiTaskRunning = false;
	vTaskDelete(NULL);
}
#endif

bool WifiManager::Tasks::StartBlinkTask(bool station, bool accessPoint, uint16_t blinkRate, uint16_t stopAfter)
{
	if (config.wifi.station.ledGpio == -1) return false;

	if (status.device.tasks.wifiBlinkTaskRunning)
	{
		StopBlinkTask();
	}

	status.device.tasks.wifiBlinkTaskRunning = true;

	uint32_t blinkMap = blinkRate;
	blinkMap |= stopAfter << 16;

	status.wifi.station.blink = station && config.wifi.station.ledGpio != -1;
	status.wifi.accessPoint.blink = accessPoint && config.wifi.accessPoint.ledGpio != -1;

	DEBUG_LOG_F("Starting Blink Task.\r\n-Wifi : %d\r\nAccess Point : %d\r\n-Rate : %dms\r\n-Stop After : %dms\r\n", station, accessPoint, blinkRate, stopAfter);


	xTaskCreatePinnedToCore(
		BlinkTask,				// Task function
		"BlinkWifi",			// Name
		512,					// Stack size
		&blinkMap,				// Parameters
		2,						// Priority
		&taskWifiBlink,			// Task handle
		1);						// Core

	return true;
}

void WifiManager::Tasks::StopBlinkTask()
{
	if (!status.device.tasks.wifiBlinkTaskRunning) return;
	DEBUG_LOG_LN("Stopping Wifi Blink Task.");

	vTaskDelete(taskWifiBlink);
	status.device.tasks.wifiBlinkTaskRunning = false;
	WifiManager::SetWifiConnectionLed();
	AccessPoint::SetAccessPointConnectionLed();

	status.wifi.station.blink = false;
	status.wifi.accessPoint.blink = false;
}

void WifiManager::Tasks::BlinkTask(void* pvParameters)
{
	
	status.device.tasks.wifiBlinkTaskRunning = true;

	uint16_t blinkRate = *((uint16_t*)pvParameters);
	uint16_t stopAfter = *(((uint16_t*)pvParameters) + 1);

	unsigned long stopTimestamp = stopAfter == 0 ? 0xFFFFFFFFFFFFFFFF : millis() + stopAfter;

	for (;;)
	{
		if(status.wifi.station.blink)
			LedToggle(config.wifi.station.ledGpio);

		if(status.wifi.accessPoint.blink)
			LedToggle(config.wifi.accessPoint.ledGpio);

		EspSense::YieldWatchdog(blinkRate);

		if (!status.device.tasks.wifiBlinkTaskRunning || millis() > stopTimestamp) break;
	}

	status.device.tasks.wifiBlinkTaskRunning = false;

	WifiManager::SetWifiConnectionLed();
	AccessPoint::SetAccessPointConnectionLed();

	status.wifi.station.blink = false;
	status.wifi.accessPoint.blink = false;

	vTaskDelete(NULL);
}


#pragma endregion
