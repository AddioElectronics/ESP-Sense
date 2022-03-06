#include "WebFileEditor.h"


#include "../WebStrings.h"
#include "../WebpageServer.h"
#include "../../../macros.h"

extern AsyncWebServer server;
extern Config_t config;
extern DeviceStatus_t status;


void Network::Website::Tools::FileEditor::Initialize()
{
	status.server.browser.tools.fileEditor.enabled = config.server.browser.tools.fileEditor && COMPILE_FILE_EDITOR;

	if (status.server.browser.tools.fileEditor.configured) return;

	status.server.browser.tools.fileEditor.configured = true;

	if (!status.server.browser.tools.fileEditor.enabled)
	{
		server.on(Strings::Urls::pageConfigToolFileEditor, HTTP_GET, [](AsyncWebServerRequest* request) {
			Network::Server::Server501(request);
		});

		return;
	}

#if COMPILE_FILE_EDITOR
	server.on(Strings::Urls::pageConfigToolFileEditor, HTTP_GET, [](AsyncWebServerRequest* request) {
		if (!status.server.authenticated)
			Network::Server::Server401(request);

		else if (status.server.browser.tools.fileEditor.enabled)
			Network::Server::ServeWebpage(Network::Website::Strings::Urls::pageConfigToolFileEditor, request);
		else
			Network::Server::Server501(request);
	});
#endif
}

#if COMPILE_FILE_EDITOR

void Network::Website::Tools::FileEditor::Enable()
{
	status.server.browser.tools.fileEditor.enabled = true;
}

void Network::Website::Tools::FileEditor::Disable()
{
	status.server.browser.tools.fileEditor.enabled = false;
}
#else
void Network::Website::Tools::FileEditor::Enable()
{

}

void Network::Website::Tools::FileEditor::Disable()
{

}
#endif


