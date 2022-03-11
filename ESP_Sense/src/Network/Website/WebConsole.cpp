#include "WebConsole.h"


#include "WebStrings.h"
#include "WebpageServer.h"
#include "../../Macros.h"

extern AsyncWebServer server;
extern Config_t config;
extern DeviceStatus_t status;


void Network::Website::Console::Initialize()
{
	status.server.browser.console.enabled = config.server.browser.console && COMPILE_SERVER_CONSOLE;

	if (status.server.browser.console.configured) return;

	status.server.browser.console.configured = true;

	if (!status.server.browser.console.enabled)
	{
		server.on(Strings::Urls::pageConsole, HTTP_GET, [](AsyncWebServerRequest* request) {
			Network::Server::Server501(request);
		});

		return;
	}

#if COMPILE_SERVER_CONSOLE
	server.on(Strings::Urls::pageConsole, HTTP_GET, [](AsyncWebServerRequest* request) {
		if (!status.server.authenticated)
			Network::Server::Server511(request);

		else if (status.server.browser.console.enabled)
			Network::Server::ServeWebpage(Network::Website::Strings::Urls::pageConsole, request);
		else
			Network::Server::Server501(request);
	});
#endif
}

#if COMPILE_SERVER_CONSOLE

void Network::Website::Console::Enable()
{
	status.server.browser.console.enabled = status.server.browser.console.configured;
}

void Network::Website::Console::Disable()
{
	status.server.browser.console.enabled = false;
}
#else
void Network::Website::console::Enable()
{

}

void Network::Website::console::Disable()
{

}
#endif
