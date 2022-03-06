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
#include <ESPAsyncWebServer.h>
#endif	/*COMPILE_SERVER*/

#if COMPILE_FTP
#include <FTPServer.h>
#include <FTPCommon.h>
#include "FtpManager.h"
#endif	/*COMPILE_FTP*/

#include "../../../ESPSense.h"
#include "../Website/WebStrings.h"
#include "../../MQTT/MqttManager.h"
#include "../../GlobalDefs.h"
#include "../../macros.h"
#include "../Website/WebStrings.h"
#include "../Website/WebpageServer.h"
#include "../OtaManager.h"

#if COMPILE_SERVER
AsyncWebServer server(80);
#endif	/*COMPILE_SERVER*/

#pragma endregion


#if COMPILE_FTP
extern FTPServer ftpServer;
#endif


extern PubSubClient mqttClient;
extern Config_t config;
extern DeviceStatus_t status;



#if COMPILE_SERVER || COMPILE_FTP
namespace Network {

	void SendAuthenticationStatus(AsyncWebServerRequest* request);
	void ParseAuthenticationData(AsyncWebServerRequest* request);
	void LogoutRequest(AsyncWebServerRequest* request);

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

#warning need to enable server, website and OTA in config mode.
#if COMPILE_SERVER
		Server::InitializeAuthenticator();
		Website::Initialize();
		OTA::Initialize();
#endif

		if (config.server.ftp.enabled || status.wifi.configMode)
		{
			Network::FTP::Construct();
			Network::FTP::SetTimeout(config.server.ftp.timeout);
			DEBUG_LOG_LN("...FTP Connection may be unstable until boot has completed...");
		}

		if (config.server.dns)
			StartDnsServer();

		//URL to call to see if server is alive.
		server.on(Website::Strings::Urls::requestAlive, HTTP_GET, [](AsyncWebServerRequest* request) {
			if (status.server.enabled)
				request->send(200);
		});

		server.begin();

		status.server.configured = true;

		//status.server.configured = true;	*Each module has its own flag
		DEBUG_LOG_LN("...Server Started.");
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
	}

	void Server::Deinitialize()
	{
		StopServer(); //Stop all modules.

		//Mark server modules as unconfigured.
		status.server.configured = false;
		status.server.authConfigured = false;
		status.server.dns.configured = false;
		memset(&status.server.browser, 0, sizeof(status.server.browser));

		//status.server.browser.configBrowser.enabled = false;
		//status.server.browser.console.enabled = false;
		//status.server.browser.mqttConfigBrowser.enabled = false;
		//status.server.browser.updater.enabled = false;
		//status.server.browser.tools.fileEditor.enabled = false;
		//status.server.browser.tools.jsonVerify.enabled = false;

		//status.server.browser.configBrowser.configured = false;
		//status.server.browser.console.configured = false;
		//status.server.browser.home.configured = false;
		//status.server.browser.mqttConfigBrowser.configured = false;
		//status.server.browser.updater.configured = false;
		//status.server.browser.tools.fileEditor.configured = false;
		//status.server.browser.tools.jsonVerify.configured = false;
	
		server.reset();	//Remove all handlers.
	}

	void Server::Loop()
	{
		//if (!status.server.enabled) return;

		Network::FTP::Loop();
		Network::OTA::Loop();

		if (status.server.sessionEnd != 0)
			if (millis() > status.server.sessionEnd)
				Logout();

	}

	void Server::Logout()
	{
		status.server.clientIP == IPAddress();
		status.server.authenticated = false;
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
		status.server.dns.configured = true;
		return;
	}

	void Server::StopDnsServer()
	{
		if (!status.server.dns.enabled) return;

		MDNS.end();
		status.server.dns.enabled = false;
	}


	void Server::InitializeAuthenticator()
	{
		//If authentication is not required, pre-set authenticated flag.
		status.server.authenticated = !config.server.authenticate;

		//Send the webpage if authentication has passed.
		server.on(Website::Strings::Urls::requestAuth, HTTP_GET, SendAuthenticationStatus);
		status.server.authConfigured = true;

		if (!config.server.authenticate) return;	//Already configured.

		//AJAX POST request from webpage, parse the data and verify.
		server.on(Website::Strings::Urls::requestAuth, HTTP_POST, ParseAuthenticationData);

		server.on(Website::Strings::Urls::requestLogout, HTTP_GET, LogoutRequest);
	}

	void SendAuthenticationStatus(AsyncWebServerRequest* request)
	{
		DEBUG_LOG_F("Sending Authentication status to browser : %d\r\n", status.server.authenticated);

		
		if ((status.server.clientIP == request->client()->remoteIP() && status.server.authenticated) || !config.server.authenticate)
			request->send_P(200, "text/plain", "Authentication Successful.");
		else
			request->send_P(401, "text/plain", "401 : Invalid username or password.");
	}

	void ParseAuthenticationData(AsyncWebServerRequest* request)
	{
		DEBUG_LOG_LN("Authentication data received. Parsing...");

		StaticJsonDocument<512> doc;

		String body = request->getParam(0)->value();

		DEBUG_LOG("-Data : ");
		DEBUG_LOG_LN(body.c_str());

		DeserializationError derror = deserializeJson(doc, body);

		if (derror)
		{
			DEBUG_LOG("Error parsing Authentication Data : ");
			DEBUG_LOG_LN(derror.c_str());
			return SendAuthenticationStatus(request);
		}
		
		char* wwwUsername = nullptr;
		char* wwwPassword = nullptr;

		if (doc.containsKey("username"))
		{
			JsonArrayConst userarray = doc["username"].as<JsonArrayConst>();

			int size = userarray.size();

			//Length is not the same, username will not match.
			//Consider not authenticated.
			if (size != config.server.user.length())
				goto Label_NotAuthenticated;

			wwwUsername = (char*)malloc(userarray.size() + 1);
			wwwUsername[size] = 0;	//Set null terminator

			DEBUG_LOG("-Encrypted Username : ");
			DEBUG_LOG_LN(wwwUsername);
			for (uint8_t i = 0; i < size; i++)
			{
				uint8_t c = userarray[i];
				wwwUsername[i] = c ^ 0xAA;
			}

			//XORArray((uint8_t*)wwwUsername, size, 0xAA);

			DEBUG_LOG("-Username : ");
			DEBUG_LOG_LN(wwwUsername);
		}

		if (doc.containsKey("password"))
		{
			JsonArrayConst passarray = doc["password"].as<JsonArrayConst>();

			int size = passarray.size();

			//Length is not the same, password will not match.
			//Consider not authenticated.
			if (size != config.server.pass.length())
				goto Label_NotAuthenticated;

			wwwPassword = (char*)malloc(passarray.size() + 1);
			wwwPassword[size] = 0;	//Set null terminator

			DEBUG_LOG("-Encrypted Password : ");
			DEBUG_LOG_LN(wwwPassword);
			for (uint8_t i = 0; i < size; i++)
			{
				uint8_t c = passarray[i];
				wwwPassword[i] = c ^ 0xAA;
			}
			
			//XORArray((uint8_t*)wwwPassword, size, 0xAA);

			DEBUG_LOG("-Password : ");
			DEBUG_LOG_LN(wwwPassword);
		}

		/*
		ArduinoJSON changes the bits while deserializing,
		must use string array.
		*/

		//if (doc.containsKey("username"))
		//{
		//	www_username = (const char*)doc["username"];
		//	DEBUG_LOG_F("Encrypted Username : %s\r\n", www_username);
		//	test(www_username.c_str(), 7);
		//	XORArray((uint8_t*)www_username.c_str(), www_username.length(), 0xAA);
		//	test(www_username.c_str(), 7);
		//	DEBUG_LOG_F("Username : %s\r\n", www_username.c_str());
		//}

		//if (doc.containsKey("password"))
		//{
		//	www_password = doc["password"].as<String>();

		//	DEBUG_LOG_F("Encrypted Password : %s\r\n", www_password.c_str());
		//	XORArray((uint8_t*)www_password.c_str(), www_password.length(), 0xAA);
		//	DEBUG_LOG_F("Password : %s\r\n", www_password.c_str());
		//}


		if (config.server.user == wwwUsername && config.server.pass == wwwPassword)
		{
			status.server.authenticated = true;
			status.server.clientIP = request->client()->remoteIP();
			status.server.sessionEnd = config.server.sessionTimeout ? millis() + (config.server.sessionTimeout * 60000) : 0;
		}

	Label_NotAuthenticated:

		if (wwwUsername != nullptr)
			free(wwwUsername);

		if (wwwPassword != nullptr)
			free(wwwPassword);

		SendAuthenticationStatus(request);
	}

	void LogoutRequest(AsyncWebServerRequest* request)
	{
		if (status.server.clientIP == request->client()->remoteIP())
		{
			Server::Logout();
			request->send_P(200, "text/plain", "Logout Successful.");
			return;
		}

		request->send(401);
	}

	


#endif
}