#pragma once

namespace Network {
		namespace Website {
	
			namespace Strings {
	

				namespace Messages {

					extern const char* unauthorized;
					extern const char* internalServerError;
					extern const char* serviceUnavailable;
					extern const char* networkAuthenticationRequried;
				}

				namespace Paths
				{
					extern const char* root;
					extern const char* rootdir;

					extern const char* defaultFile;

					extern const char* pageHome;
					extern const char* pageConfig;
					extern const char* pageWebUpdate;
					extern const char* pageWebUpdating;
					extern const char* pageConsole;

					extern const char* pageTools;
					extern const char* pageConfigToolFileEditor;
					extern const char* pageConfigToolJsonVerifier;
					extern const char* pageConfigToolTemplateGen;

					//extern const char* pageMqtt;
					extern const char* pageMqttDevices;
					extern const char* pageMqttBinarySensors;
					extern const char* pageMqttSensors;
					extern const char* pageMqttButtons;
					extern const char* pageMqttLights;
					extern const char* pageMqttSwitches;


					extern const char* errorPattern;
					extern const char* error401;
					extern const char* error404;
					extern const char* error501;
					extern const char* error511;

					extern const char* assetsJS;
					extern const char* assetsCSS;
					extern const char* assetsIMG;

					extern const char* assetsSensorsJS;
					extern const char* assetsBinarySensorsJS;
					extern const char* assetsButtonsJS;
					extern const char* assetsLightsJS;
					extern const char* assetsSwitchesJS;
				}
	
				namespace Urls {

					
					extern const char* defaultFile;
	
					extern const char* pageHome;
					extern const char* pageConfig;
					extern const char* pageWebUpdate;
					extern const char* pageConsole;
	
					extern const char* pageTools;
					extern const char* pageConfigToolFileEditor;
					extern const char* pageConfigToolJsonVerifier;
					extern const char* pageConfigToolTemplateGen;

					//extern const char* pageMqtt;
					extern const char* pageMqttDevices;
					extern const char* pageMqttBinarySensors;
					extern const char* pageMqttSensors;
					extern const char* pageMqttButtons;
					extern const char* pageMqttLights;
					extern const char* pageMqttSwitches;

					extern const char* requestVersion;
					extern const char* requestAuth;
					extern const char* requestLogout;
					extern const char* requestWebUpdate;
					extern const char* requestReset;
					extern const char* requestAlive;
					extern const char* requestStatus;
					extern const char* requestMqttDeviceInfo;
	
					extern const char* error401;
					extern const char* error404;
					extern const char* error501;
					extern const char* error511;

					extern const char* assetsJS;
					extern const char* assetsCSS;
					extern const char* assetsIMG;

					extern const char* assetsSensorsJS;
					extern const char* assetsBinarySensorsJS;
					extern const char* assetsButtonsJS;
					extern const char* assetsLightsJS;
					extern const char* assetsSwitchesJS;
				}

				namespace ContentType {

					extern const char* textPlain;
					extern const char* textHTML;
					extern const char* textJSON;
					extern const char* textCSS;
					//extern const char* textJS = "text/javascript "; //Obsolete
					extern const char* textXML;
	
					extern const char* imageJPG;
					extern const char* imagePNG;
					extern const char* imageSVG;
	
					extern const char* appJSON;
					extern const char* appJS;
					extern const char* appXML;
				}
			}
		}
	}

//namespace Network {
//	namespace Website {
//
//		class Strings {
//
//		public:
//			class Messages {
//			public:
//				static const char* unauthorized;
//			};
//
//			class Urls {
//			public:
//				static const char* root;
//
//				static const char* pageHome;
//				static const char* pageConfig;
//				static const char* pageConfigMqtt;
//				static const char* pageWebUpdate;
//				static const char* pageConsole;
//
//				static const char* pageTools;
//				static const char* pageConfigToolFileEditor;
//				static const char* pageConfigToolJsonVerifier;
//
//				static const char* requestWebUpdate;
//
//				static const char* error401;
//				static const char* error404;
//				static const char* error501;
//
//			};
//
//			class ContentType {
//			public:
//				static const char* textPlain;
//				static const char* textHTML;
//				static const char* textJSON;
//				static const char* textCSS;
//				//static const char* textJS = "text/javascript "; //Obsolete
//				static const char* textXML;
//
//				static const char* imageJPG;
//				static const char* imagePNG;
//				static const char* imageSVG;
//
//				static const char* appJSON;
//				static const char* appJS;
//				static const char* appXML;
//			};
//		};
//	}
//}