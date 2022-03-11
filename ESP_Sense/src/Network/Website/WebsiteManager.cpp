#include "WebsiteManager.h"

#warning reminder to set all error pages to the same page, but dynamically change message.And remove nagivationand other global elements from html fileand add with JS.

#if COMPILE_SERVER
#pragma region Includes

#include <ArduinoJson.hpp>
#include <ArduinoJson.h>

#if defined(ESP8266)
#include <Hash.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#if COMPILE_SERVER
#include <ESPAsyncTCP.h>
#endif	/*COMPILE_SERVER*/
#elif defined(ESP32)
#include <esp32-hal-uart.h>
#include <WiFi.h>
#include <FS.h>
#include <LITTLEFS.h>
#if COMPILE_SERVER
#include <WiFiUdp.h>
#include <AsyncTCP.h>
#endif	/*COMPILE_SERVER*/
#endif	/*defined(ESP8266) else defined(ESP32)*/

#if COMPILE_SERVER
#include <ESPAsyncWebServer.h>
#endif	/*COMPILE_SERVER*/

#if COMPILE_OTA
#include <ArduinoOTA.h>	
#endif	/*COMPILE_OTA*/

#if COMPILE_FTP
#include <FTPServer.h>
#include <FTPCommon.h>
#include "../Server/FtpManager.h"
#endif	/*COMPILE_FTP*/


#include "../../../ESP_Sense.h"
#include "../../Config/ConfigManager.h"
#include "../WifiManager.h"
#include "../../MQTT/MqttManager.h"

#include "../../GlobalDefs.h"
#include "../../macros.h"

#include "WebStrings.h"
#include "WebpageServer.h"
#include "WebUpdater.h"
#include "WebConsole.h"
#include "WebConfigBrowser.h"
#include "Tools/WebFileEditor.h"
#include "Tools/WebJsonValidator.h"


#if COMPILE_SERVER
extern AsyncWebServer server;
#endif

#pragma endregion

extern Config_t config;
extern DeviceStatus_t status;
extern StatusRetained_t statusRetained;

extern HardwareSerial* serial;			//Message
extern HardwareSerial* serialDebug;		//Debug

namespace Network {
	namespace Website
	{
		void HandlerNotFound(AsyncWebServerRequest* request);		
		const char* GetContentType(const String& url);

		

		void Initialize()
		{
			DEBUG_LOG_LN("Initializing Website...");
			status.server.browser.enabled = true;
			Website::WebUpdate::Initialize();
			Website::ConfigBrowser::Initialize();
			Website::ConfigBrowser::InitializeMqtt();
			Website::Console::Initialize();
			Website::Tools::FileEditor::Initialize();
			Website::Tools::JsonValidator::Initialize();

			if (status.server.browser.configured) return;

			server.serveStatic("/", ESP_FS, Strings::Paths::rootdir).setDefaultFile(Strings::Paths::defaultFile);
			//server.serveStatic("/", ESP_FS, Strings::Paths::pageHome);
			//server.serveStatic(Strings::Urls::pageHome, ESP_FS, Strings::Paths::pageHome);
			//server.serveStatic("/", ESP_FS, Strings::Paths::rootdir).setDefaultFile(homePath.c_str());


			//Serve all CSS, JS and IMG files.
			server.serveStatic(Strings::Urls::assetsCSS, ESP_FS, Strings::Paths::assetsCSS);
			server.serveStatic(Strings::Urls::assetsJS, ESP_FS, Strings::Paths::assetsJS);
			server.serveStatic(Strings::Urls::assetsIMG, ESP_FS, Strings::Paths::assetsIMG);

			//Serve MQTT device JS assets
			server.serveStatic(Strings::Urls::assetsBinarySensorsJS, ESP_FS, Strings::Paths::assetsBinarySensorsJS);
			server.serveStatic(Strings::Urls::assetsButtonsJS, ESP_FS, Strings::Paths::assetsButtonsJS);
			server.serveStatic(Strings::Urls::assetsSensorsJS, ESP_FS, Strings::Paths::assetsSensorsJS);
			server.serveStatic(Strings::Urls::assetsLightsJS, ESP_FS, Strings::Paths::assetsLightsJS);
			server.serveStatic(Strings::Urls::assetsSwitchesJS, ESP_FS, Strings::Paths::assetsSwitchesJS);

			server.onNotFound(HandlerNotFound);			

			status.server.browser.configured = true;
			DEBUG_LOG_LN("...Website Initialized.\r\n");
		}


		void StopHosting()
		{
			if (status.server.browser.enabled)
				status.server.browser.configBrowser.enabled = false;

			if (status.server.browser.configBrowser.enabled)
				status.server.browser.configBrowser.enabled = false;

			if (status.server.browser.console.enabled)
				status.server.browser.console.enabled = false;

			if (status.server.browser.mqttDevices.enabled)
				status.server.browser.mqttDevices.enabled = false;

			if (status.server.browser.updater.enabled)
				status.server.browser.updater.enabled = false;

			if (status.server.browser.tools.fileEditor.enabled)
				status.server.browser.tools.fileEditor.enabled = false;

			if (status.server.browser.tools.jsonVerify.enabled)
				status.server.browser.tools.jsonVerify.enabled = false;
		}



		const char* GetContentType(const String& url)
		{
			DEBUG_LOG_F("Getting Content Type from url : %s\r\n", url.c_str());

			int index = url.lastIndexOf('.');

			if (index == -1) 
			{
				DEBUG_LOG_LN("...Not a file");
				return nullptr;
			}

			String ext = url.substring(index + 1);
			ext.toLowerCase();

			//DEBUG_LOG_F("Extension : %s\r\n", ext.c_str());

			
			
			if (ext == "html")
			{
				///*DEBUG_LOG_LN("html");
				//return "text/html";*/

				//DEBUG_LOG_LN("HTML Files are forbidden, as certain features decide what pages to serve.");

				////Do not automatically serve HTML pages,
				////certain features may be disabled.
				//return nullptr;
				return Strings::ContentType::textHTML;
			}
			if (ext == "css")
			{
				return Strings::ContentType::textCSS;
			}
			else if (ext == "js")
			{
				return Strings::ContentType::appJS;
			}
			else if (ext == "json")
			{
				return Strings::ContentType::appJSON;
			}
			else if (ext == "xml")
			{
				return Strings::ContentType::appXML;
			}
			else if (ext == "png")
			{
				return Strings::ContentType::imagePNG;
			}
			else if (ext == "jpg")
			{
				return Strings::ContentType::imageJPG;
			}
			else if (ext == "svg")
			{
				return Strings::ContentType::imageSVG;
			}
			else if (ext == "txt")
			{
				return Strings::ContentType::textPlain;
			}
			else
			{
				DEBUG_LOG_LN("Invalid content type.");
				return nullptr;
			}
		}


		void HandlerNotFound(AsyncWebServerRequest* request)
		{
			DEBUG_LOG_LN("Server handler Not Found. Searching for file...");

			String reqUrl = request->url();

			const char* contentType = GetContentType(reqUrl);

			String filepath = Strings::Paths::root + reqUrl;
			DEBUG_LOG_F("Filepath : %s\r\n", filepath.c_str());

			//if (filepath == Strings::Urls::root)
			//	filepath = Strings::Urls::pageHome;

			//Redirect to 404
			/*if (strcmp(contentType, "html") == 0)*/
			if(contentType == nullptr)
				filepath = Strings::Urls::error404;

			if (ESP_FS.exists(filepath.c_str())) {
				DEBUG_LOG_LN("exists");
				request->send(ESP_FS, filepath);
			}
			else {
				request->send_P(404, "text/plain", "Not found");
			}

			DEBUG_NEWLINE();
		}
	}
}
#endif