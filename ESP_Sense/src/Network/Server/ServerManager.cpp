#include "ServerManager.h"

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
#endif	/*COMPILE_SERVER*/
#elif defined(ESP32)
#include <esp32-hal-uart.h>
#include <esp_err.h>
//#include <WiFi.h>
#include <FS.h>
//#include <SPIFFS.h>
#include <LITTLEFS.h>
//#define LittleFS LITTLEFS
#if COMPILE_SERVER
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#endif	/*COMPILE_SERVER*/
#endif	/*defined(ESP8266) else defined(ESP32)*/

#if COMPILE_SERVER
#include "../Website/WebsiteManager.h"
#endif	/*COMPILE_SERVER*/

#if COMPILE_FTP
#include <FTPServer.h>
#include <FTPCommon.h>
#include "FtpManager.h"
#endif	/*COMPILE_FTP*/

#include "../../../ESP_Sense.h"
#include "../Website/WebStrings.h"
#include "../../MQTT/MqttManager.h"
#include "../../GlobalDefs.h"
#include "../../macros.h"
#include "../Website/WebStrings.h"
#include "../Website/WebpageServer.h"
#include "../OtaManager.h"
#include "Authentication.h"
#include "SpecialRequests.h"

#if COMPILE_SERVER
AsyncWebServer server(80);
//AsyncEventSource events("/events");
#endif	/*COMPILE_SERVER*/

#pragma endregion


#if COMPILE_FTP
extern FTPServer ftpServer;
#endif


extern PubSubClient mqttClient;
extern Config_t config;
extern GlobalStatus_t status;



#if COMPILE_SERVER || COMPILE_FTP
namespace Network {



	void Server::Initialize()
	{
		DEBUG_LOG_LN("Starting Server...");
		status.server.enabled = true;

		if (status.server.configured)
		{
			//Already configured, need to call denit first.
			DEBUG_LOG_LN("...Server already configured. If trying to reconfigure, it needs to be deinitialized first.");
			return;
		}

		#warning need to enable server, websiteand OTA in config mode.
#if COMPILE_SERVER
		Authentication::Initialize();
		SpecialRequests::Initialize();
		Website::Initialize();
		OTA::Initialize();
#endif

		if (config.server.ftp.enabled || status.device.configMode)
		{
			Network::FTP::Construct();
			Network::FTP::SetTimeout(config.server.ftp.timeout);
			DEBUG_LOG_LN("...FTP Connection may be unstable until boot has completed...");
		}

		if (config.server.dns)
			StartDnsServer();

		server.begin();

		status.server.configured = true;

		//status.server.configured = true;	*Each module has its own flag
		DEBUG_LOG_LN("...Server Started.\r\n\r\n\r\n");
	}

	void Server::StartServer()
	{

		if (!status.server.configured) Initialize();

		status.server.enabled = true;
	}

	void Server::StopServer()
	{
		//Already stopped.
		if (!status.server.enabled) return;

		DEBUG_LOG_LN("Stopping Server...");

		StopDnsServer();
		Website::StopHosting();
		OTA::Disable();
		Network::FTP::Stop();

		server.end();

		status.server.enabled = false;
		DEBUG_LOG_LN("...Server Stopped.");
		DEBUG_NEWLINE();
	}

	void Server::Deinitialize()
	{
		DEBUG_LOG_LN("Deinitializing Server...");
		StopServer(); //Stop all modules.

		//Mark server modules as unconfigured.
		status.server.configured = false;
		status.server.authConfigured = false;
		status.server.dns.configured = false;
		memset(&status.server.browser, 0, sizeof(status.server.browser));

		//status.server.browser.configBrowser.enabled = false;
		//status.server.browser.console.enabled = false;
		//status.server.browser.mqttDevices.enabled = false;
		//status.server.browser.updater.enabled = false;
		//status.server.browser.tools.fileEditor.enabled = false;
		//status.server.browser.tools.jsonVerify.enabled = false;

		//status.server.browser.configBrowser.configured = false;
		//status.server.browser.console.configured = false;
		//status.server.browser.home.configured = false;
		//status.server.browser.mqttDevices.configured = false;
		//status.server.browser.updater.configured = false;
		//status.server.browser.tools.fileEditor.configured = false;
		//status.server.browser.tools.jsonVerify.configured = false;
	
		server.reset();	//Remove all handlers.

		DEBUG_LOG_LN("...Server Deinitialized.\r\n");
	}

	void Server::Loop()
	{
		//if (!status.server.enabled) return;

		Network::FTP::Loop();
		Network::OTA::Loop();

		if(status.server.authenticated)
		if (status.server.sessionEnd != 0)
			if (millis() > status.server.sessionEnd)
				Authentication::Logout();

	}



	void Server::StartDnsServer()
	{
		if (status.server.dns.configured) return;

		DEBUG_LOG_LN("Starting DNS Server...");
		status.server.dns.enabled = MDNS.begin(config.server.hostname.c_str());

		if (!status.server.dns.enabled)
		{
			DEBUG_LOG_LN("...DNS Server Failed to start.");
			return;
		}

		DEBUG_LOG_LN("Adding Service : http tcp 80");
		MDNS.addService("http", "tcp", 80);


		if (status.server.ftp.enabled)
		{
			DEBUG_LOG_LN("Adding Service : ftp tcp 21");
			//DEBUG_LOG_LN("Adding Service : ftp udp 8266");
			MDNS.addService("ftp", "tcp", 21);
			//MDNS.addService("ftp", "udp", 8266);
		}

		if (status.server.ota.enabled /*|| config.server.updater.mode == COM_UPDATER || config.server.updater.mode == HTTP_AND_COM_UPDATER*/)
		{
			DEBUG_LOG_LN("Adding Service : Arduino");
			MDNS.enableArduino();
		}

		DEBUG_LOG_LN("...DNS Started.");
		DEBUG_NEWLINE();
		status.server.dns.configured = true;
		return;
	}

	void Server::StopDnsServer()
	{
		if (!status.server.dns.enabled) return;

		DEBUG_LOG_LN("Stopping DNS Server.");

		MDNS.end();
		status.server.dns.enabled = false;
	}





#endif
}