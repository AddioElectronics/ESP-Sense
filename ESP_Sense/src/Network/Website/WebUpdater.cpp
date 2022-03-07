#include "WebUpdater.h"

#include <Update.h>

#include "../../../ESP_Sense.h"
#include "WebStrings.h"
#include "WebpageServer.h"
#include "../../macros.h"
#include "../Server/Authentication.h"


extern AsyncWebServer server;
extern Config_t config;
extern DeviceStatus_t status;

size_t totalUploadSize = 0;

void HandleUpdate(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final);

void Network::Website::WebUpdate::Initialize()
{
	status.server.browser.updater.enabled = config.server.browser.updater;

	if (status.server.browser.updater.configured) return;

	status.server.browser.updater.configured = true;

	if (!status.server.browser.updater.enabled)
	{
		server.on(Strings::Urls::pageWebUpdate, HTTP_GET, [](AsyncWebServerRequest* request) {
			Network::Server::Server501(request);
		});

		return;
	}

#if COMPILE_WEBUPDATE
	server.on(Strings::Urls::pageWebUpdate, HTTP_GET, [](AsyncWebServerRequest* request) {
		if (!Server::Authentication::IsAuthenticated(request))
			Network::Server::Server401(request);
		else if (status.server.browser.updater.enabled)
			Network::Server::ServeWebpage(Network::Website::Strings::Urls::pageWebUpdate, request);
		else
			Network::Server::Server501(request);
	});


	//File upload
	server.on(Strings::Urls::requestWebUpdate, HTTP_POST, [](AsyncWebServerRequest* request) {
		if (!Server::Authentication::IsAuthenticated(request))
		{
			request->send(401, Strings::ContentType::textPlain, Strings::Messages::unauthorized);
			return;
		}
		request->send(200);
	}, HandleUpdate);
#endif
}

#if COMPILE_WEBUPDATE

void Network::Website::WebUpdate::Enable()
{
	status.server.browser.updater.enabled = status.server.browser.updater.configured;
}

void Network::Website::WebUpdate::Disable()
{
	status.server.browser.updater.enabled = false;
}

void HandleUpdate(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
	if (!Network::Server::Authentication::IsAuthenticated(request)) return;

	#warning reminder to disable MQTT devicesand other server modules.

	status.server.browser.updater.updating = true;

	if (status.server.updating != UpdateMode::ESP_UPDATE_NULL)
		EspSense::EnterUpdateMode(UpdateMode::ESP_UPDATE_WEB);

	String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
	DEBUG_LOG_LN(logmessage);

	if (!index) {
		DEBUG_LOG_LN("Firmware Update Started(Web Update)...");
		DEBUG_LOG_F("Filename: %s\n", filename.c_str());
		serialDebug->setDebugOutput(true);
		totalUploadSize = 0;

		#warning remember to verifyand set size dependant on FLASHSIZE

		if (!Update.begin())
		{
			Update.printError(*serialDebug);
		}
	}

	if (len) {
		size_t written = Update.write(data, len);
		totalUploadSize += written;
		if (written != len)
		{
			Update.printError(*serialDebug);
		}
	}

	if (final) {
		if (Update.end(true)) { //true to set the size to the current progress
			DEBUG_LOG_F("Update Success : %u bytes \r\nRebooting...\r\n", totalUploadSize);
			status.server.updating = UpdateMode::ESP_UPDATE_NULL;
			EspSense::RestartDevice();
		}
		else
		{
			Update.printError(*serialDebug);
		}
		serialDebug->setDebugOutput(false);
	}

}
#else
void Network::Website::WebUpdate::Enable()
{

}

void Network::Website::WebUpdate::Disable()
{

}
#endif


