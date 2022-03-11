#include "WebJsonValidator.h"


#include "../WebStrings.h"
#include "../WebpageServer.h"
#include "../../../macros.h"

extern AsyncWebServer server;
extern Config_t config;
extern DeviceStatus_t status;


void Network::Website::Tools::JsonValidator::Initialize()
{
	status.server.browser.tools.jsonVerify.enabled = config.server.browser.tools.jsonVerify && COMPILE_WEB_JSON_VALIDATOR;

	if (status.server.browser.tools.jsonVerify.configured) return;

	status.server.browser.tools.jsonVerify.configured = true;

	if (!status.server.browser.tools.jsonVerify.enabled)
	{
		server.on(Strings::Urls::pageConfigToolJsonVerifier, HTTP_GET, [](AsyncWebServerRequest* request) {
			Network::Server::Server501(request);
		});

		return;
	}

#if COMPILE_WEB_JSON_VALIDATOR
	server.on(Strings::Urls::pageConfigToolJsonVerifier, HTTP_GET, [](AsyncWebServerRequest* request) {
		if (!status.server.authenticated)
			Network::Server::Server511(request);

		else if (status.server.browser.tools.jsonVerify.enabled)
			Network::Server::ServeWebpage(Network::Website::Strings::Urls::pageConfigToolJsonVerifier, request);
		else
			Network::Server::Server501(request);
	});
#endif
}

#if COMPILE_WEB_JSON_VALIDATOR

void Network::Website::Tools::JsonValidator::Enable()
{
	status.server.browser.tools.jsonVerify.enabled = true;
}

void Network::Website::Tools::JsonValidator::Disable()
{
	status.server.browser.tools.jsonVerify.enabled = false;
}
#else
void Network::Website::Tools::JsonValidator::Enable()
{

}

void Network::Website::Tools::JsonValidator::Disable()
{

}
#endif

