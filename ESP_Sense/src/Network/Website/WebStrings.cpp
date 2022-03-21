#include "WebStrings.h"

namespace Network {
	namespace Website {
		namespace Strings {

			namespace Headers {

				const char* defaultHeaderName = "Server";
				const char* defaultHeaderValue = "ESP Sense";
			}

			namespace Messages {

				const char* badRequest = "400 : Bad Request";
				const char* unauthorized = "401 : Unauthorized";
				const char* internalServerError = "500 : Internal Server Error";
				const char* serviceUnavailable = "503 : Service Unavailable";
				const char* networkAuthenticationRequried = "511 : Network Authentication Required";
			}

			namespace Paths
			{
				const char* root  = "/www";
				const char* rootdir = "/www/";

				const char* defaultFile = "index.html";

				const char* pageHome = "/www/index.html";
				const char* pageConfig = "/www/config.html";
				const char* pageWebUpdate = "/www/update.html";
				const char* pageUpdating = "/www/update/updating.html";
				const char* pageConsole = "/www/console.html";

				const char* pageTools = "/www/tools.html";
				const char* pageConfigToolFileEditor = "/www/tools/fileeditor.html";
				const char* pageConfigToolJsonVerifier = "/www/tools/jsonass.html";
				const char* pageConfigToolTemplateGen = "/www/tools/templategen.html";

				//const char* pageMqtt = "/www/mqtt.html";
				const char* pageMqttDevices = "/www/mqtt/devices.html";
				const char* pageMqttBinarySensors = "/www/mqtt/devices/binarySensors.html";
				const char* pageMqttSensors = "/www/mqtt/devices/sensors.html";
				const char* pageMqttButtons = "/www/mqtt/devices/buttons.html";
				const char* pageMqttLights = "/www/mqtt/devices/lights.html";
				const char* pageMqttSwitches = "/www/mqtt/devices/switches.html";


				const char* errorPattern = "/www/error/%00d.html";
				const char* error401 = "/www/error/401.html";
				const char* error404 = "/www/error/404.html";
				const char* error501 = "/www/error/501.html";
				const char* error511 = "/www/error/511.html";

				const char* assetsJS = "/www/assets/js/";
				const char* assetsCSS = "/www/assets/css/";
				const char* assetsIMG = "/www/assets/img/";

				const char* assetsSensorsJS = "/www/assets/js/mqtt/devices/sensors";
				const char* assetsBinarySensorsJS = "/www/assets/js/mqtt/devices/binarysensors";
				const char* assetsButtonsJS = "/www/assets/js/mqtt/devices/buttons";
				const char* assetsLightsJS = "/www/assets/js/mqtt/devices/lights";
				const char* assetsSwitchesJS = "/www/assets/js/mqtt/devices/switches";
			}

			namespace Urls {

				const char* pageHome = Paths::pageHome + 4; 
				const char* pageConfig = Paths::pageConfig + 4;
				const char* pageWebUpdate = Paths::pageWebUpdate + 4;
				const char* pageConsole = Paths::pageConsole + 4;

				const char* pageTools = Paths::pageTools + 4;
				const char* pageConfigToolFileEditor = Paths::pageConfigToolFileEditor + 4;
				const char* pageConfigToolJsonVerifier = Paths::pageConfigToolJsonVerifier + 4;
				const char* pageConfigToolTemplateGen = Paths::pageConfigToolTemplateGen + 4;

				//const char* pageMqtt = Paths::pageMqtt + 4;
				const char* pageMqttDevices = Paths::pageMqttDevices + 4;
				const char* pageMqttBinarySensors = Paths::pageConsole + 4;
				const char* pageMqttSensors = Paths::pageMqttSensors + 4;
				const char* pageMqttButtons = Paths::pageMqttButtons + 4;
				const char* pageMqttLights = Paths::pageMqttLights + 4;
				const char* pageMqttSwitches = Paths::pageMqttSwitches + 4;

				const char* requestVersion = "/version";
				const char* requestAuth = "/auth";
				const char* requestLogout = "/logout";
				const char* requestWebUpdate = "/update/upload";
				const char* requestReset = "/reset";
				const char* requestAlive = "/alive";
				const char* requestStatus = "/status"; 
				const char* requestMqttDeviceInfo = "/mqtt/devices/info";
				const char* requestCookie = "/mqtt/devices/info";
				const char* requestControl = "/control";

				const char* error401 = Paths::error401 + 4;
				const char* error404 = Paths::error404 + 4;
				const char* error501 = Paths::error501 + 4;
				const char* error511 = Paths::error511 + 4;

				const char* assetsJS = Paths::assetsJS + 4;
				const char* assetsCSS = Paths::assetsCSS + 4;
				const char* assetsIMG = Paths::assetsIMG + 4;

				const char* assetsSensorsJS = Paths::assetsSensorsJS + 4;
				const char* assetsBinarySensorsJS = Paths::assetsBinarySensorsJS + 4;
				const char* assetsButtonsJS = Paths::assetsButtonsJS + 4;
				const char* assetsLightsJS = Paths::assetsLightsJS + 4;
				const char* assetsSwitchesJS = Paths::assetsSwitchesJS + 4;
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