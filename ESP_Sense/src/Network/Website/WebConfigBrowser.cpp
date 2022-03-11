#include "WebConfigBrowser.h"

#include "WebStrings.h"
#include "WebpageServer.h"
#include "../../Macros.h"



extern AsyncWebServer server;
extern Config_t config;
extern DeviceStatus_t status;

extern AsyncWebServer server;

extern Config_t config;
extern DeviceStatus_t status;

void Network::Website::ConfigBrowser::Initialize()
{
	status.server.browser.configBrowser.enabled = config.server.browser.config && COMPILE_CONFIG_BROWSER;

	if (status.server.browser.configBrowser.configured) return;		//Already configured, exit.
	status.server.browser.configBrowser.configured = true;			//Set configured flag

	//Server 501 if browser is not enabled in config..
	if (!status.server.browser.configBrowser.enabled)
	{
		server.on(Strings::Urls::pageConfig, HTTP_GET, [](AsyncWebServerRequest* request) {
			Network::Server::Server501(request);
		});

		return;
	}

#if COMPILE_CONFIG_BROWSER
	status.server.browser.configBrowser.enabled = true;


	server.on(Website::Strings::Urls::pageConfig, HTTP_GET, [](AsyncWebServerRequest* request) {
		if (status.server.browser.configBrowser.enabled)
			Network::Server::ServeWebpage(Website::Strings::Urls::pageConfig, request);
		else
			Network::Server::Server501(request);
	});

	status.server.browser.configBrowser.configured = true;
#endif
}

void Network::Website::ConfigBrowser::InitializeMqtt()
{
	status.server.browser.mqttDevices.enabled = config.server.browser.mqttDevices && COMPILE_CONFIG_BROWSER_MQTT;

	if (status.server.browser.mqttDevices.configured) return;		//Already configured, exit.
	status.server.browser.mqttDevices.configured = true;			//Set configured flag

	//Server 501 if browser is not enabled in config.
	if (!status.server.browser.mqttDevices.enabled)
	{
#warning not checking auth
		server.on(Strings::Urls::pageMqttDevices, HTTP_GET, [](AsyncWebServerRequest* request) {
			Network::Server::Server501(request);
		});

		return;
	}

#if COMPILE_CONFIG_BROWSER_MQTT
	status.server.browser.mqttDevices.enabled = true;


	server.on(Website::Strings::Urls::pageMqttDevices, HTTP_GET, [](AsyncWebServerRequest* request) {
		if (status.server.browser.mqttDevices.enabled)
			Network::Server::ServeWebpage(Website::Strings::Urls::pageMqttDevices, request);
		else
			Network::Server::ServeWebpage(Website::Strings::Urls::error501, request);
	});

	status.server.browser.mqttDevices.configured = true;
#endif
}

void Network::Website::ConfigBrowser::Enable()
{
	status.server.browser.configBrowser.enabled = status.server.browser.configBrowser.configured;
}

void Network::Website::ConfigBrowser::Disable()
{
	status.server.browser.configBrowser.enabled = false;
}

void Network::Website::ConfigBrowser::EnableMqtt()
{
	status.server.browser.mqttDevices.enabled = status.server.browser.mqttDevices.configured;
}

void Network::Website::ConfigBrowser::DisableMqtt()
{
	status.server.browser.mqttDevices.enabled = false;
}

