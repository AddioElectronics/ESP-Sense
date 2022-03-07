#include "WebStrings.h"

namespace Network {
	namespace Website {
		namespace Strings {

			namespace Messages {

				const char* unauthorized = "401 : Unauthorized";
			}

			namespace Paths
			{
				const char* root  = "/www";
				const char* rootdir = "/www/";

				const char* assetsJS = "/www/assets/js/";
				const char* assetsCSS = "/www/assets/css/";
				const char* assetsIMG = "/www/assets/img/";

				const char* pageHome = "/www/index.html";
				const char* pageConfig = "/www/config.html";
				const char* pageConfigMqtt = "/www/config/mqtt.html";
				const char* pageWebUpdate = "/www/update.html";
				const char* pageConsole = "/www/console.html";

				const char* pageTools = "/www/tools.html";
				const char* pageConfigToolFileEditor = "/www/tools/fileeditor.html";
				const char* pageConfigToolJsonVerifier = "/www/tools/jsonass.html";
				const char* pageConfigToolTemplateGen = "/www/tools/templategen.html";

				const char* requestAuth = "/www/auth";
				const char* requestLogout = "/www/logout";
				const char* requestWebUpdate = "/www/update/upload";
				const char* requestReset = "/www/reset";
				const char* requestAlive = "/www/alive";

				const char* error401 = "/www/error/401.html";
				const char* error404 = "/www/error/404.html";
				const char* error501 = "/www/error/501.html";
			}

			namespace Urls {

				const char* pageHome = Paths::pageHome + 5; 
				const char* pageConfig = Paths::pageConfig + 5;
				const char* pageConfigMqtt = Paths::pageConfigMqtt + 5;
				const char* pageWebUpdate = Paths::pageWebUpdate + 5;
				const char* pageConsole = Paths::pageConsole + 5;

				const char* pageTools = Paths::pageTools + 5;
				const char* pageConfigToolFileEditor = Paths::pageConfigToolFileEditor + 5;
				const char* pageConfigToolJsonVerifier = Paths::pageConfigToolJsonVerifier + 5;
				const char* pageConfigToolTemplateGen = Paths::pageConfigToolTemplateGen + 5;

				const char* requestAuth = Paths::requestAuth + 5;
				const char* requestLogout = Paths::requestLogout + 5;
				const char* requestWebUpdate = Paths::requestWebUpdate + 5;
				const char* requestReset = Paths::requestReset + 5;
				const char* requestAlive = Paths::requestAlive + 5;

				const char* error401 = Paths::error401 + 5;
				const char* error404 = Paths::error404 + 5;
				const char* error501 = Paths::error501 + 5;

				const char* assetsJS = Paths::assetsJS + 5;
				const char* assetsCSS = Paths::assetsCSS + 5;
				const char* assetsIMG = Paths::assetsIMG + 5;

				/*const char* pageHome = "/index.html";
				const char* pageConfig = "/config.html";
				const char* pageConfigMqtt = "/config/mqtt.html";
				const char* pageWebUpdate = "/update.html";
				const char* pageConsole = "/console.html";

				const char* pageTools = "/tools.html";
				const char* pageConfigToolFileEditor = "/tools/fileeditor.html";
				const char* pageConfigToolJsonVerifier = "/tools/jsonass.html";
				const char* pageConfigToolTemplateGen = "/tools/templategen.html";

				const char* requestAuth = "/auth";
				const char* requestLogout = "/logout";
				const char* requestWebUpdate = "/update/upload";
				const char* requestReset = "/reset";
				const char* requestAlive = "/alive";

				const char* error401 = "/error/401.html";
				const char* error404 = "/error/404.html";
				const char* error501 = "/error/501.html";

				const char* assetsJS = "/assets/js/";
				const char* assetsCSS = "/assets/css/";
				const char* assetsIMG = "/assets/img/";*/


			}

			

			namespace ContentType {

				const char* textPlain = "text/plain";
				const char* textHTML = "text/html";
				const char* textJSON = "text/json";
				const char* textCSS = "text/css";
				const char* textXML = "text/xml";

				const char* imageJPG = "image/jpeg";
				const char* imagePNG = "image/png";
				const char* imageSVG = "image/svg+xml";

				const char* appJSON = "application/json";
				const char* appJS = "application/javascript";
				const char* appXML = "application/xml";
			}
		}
	}
}
//
//#pragma region Messages
//
//const char* Network::Website::Strings::Messages::unauthorized = "401 : Unauthorized";
//
//#pragma endregion
//
//#pragma region Urls
//
//const char* Network::Website::Strings::Urls::root = "/www";
//
//const char* Network::Website::Strings::Urls::pageHome = "/index.html";
//const char* Network::Website::Strings::Urls::pageConfig = "/config/index.html";
//const char* Network::Website::Strings::Urls::pageConfigMqtt = "/config/mqtt/index.html";
//const char* Network::Website::Strings::Urls::pageWebUpdate = "/update/index.html";
//const char* Network::Website::Strings::Urls::pageConsole = "/console/index.html";
//
//const char* Network::Website::Strings::Urls::pageTools = "/tools/index.html";
//const char* Network::Website::Strings::Urls::pageConfigToolFileEditor = "/config/tools/fileeditor/index.html";
//const char* Network::Website::Strings::Urls::pageConfigToolJsonVerifier = "/config/tools/json/index.html";
//
//const char* Network::Website::Strings::Urls::requestWebUpdate = "/update/upload";
//
//const char* Network::Website::Strings::Urls::error401 = "/error/401.html";
//const char* Network::Website::Strings::Urls::error404 = "/error/404.html";
//const char* Network::Website::Strings::Urls::error501 = "/error/501.html";
//
//#pragma endregion
//
//
//#pragma region ContentType
//
//const char* Network::Website::Strings::ContentType::textPlain = "text/plain";
//const char* Network::Website::Strings::ContentType::textHTML = "text/html";
//const char* Network::Website::Strings::ContentType::textJSON = "text/json";
//const char* Network::Website::Strings::ContentType::textCSS = "text/css";
//const char* Network::Website::Strings::ContentType::textXML = "text/xml";
//
//const char* Network::Website::Strings::ContentType::imageJPG = "image/jpeg";
//const char* Network::Website::Strings::ContentType::imagePNG = "image/png";
//const char* Network::Website::Strings::ContentType::imageSVG = "image/svg+xml";
//
//const char* Network::Website::Strings::ContentType::appJSON = "application/json";
//const char* Network::Website::Strings::ContentType::appJS = "application/javascript";
//const char* Network::Website::Strings::ContentType::appXML = "application/xml";
//
//#pragma endregion