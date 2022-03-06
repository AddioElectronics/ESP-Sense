#ifndef _WEBCONFIGBROWSER_h
#define _WEBCONFIGBROWSER_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include "../../Config/config_master.h"

#if COMPILE_CONFIG_BROWSER
namespace Network {
	namespace Website {
		namespace ConfigBrowser {
			void Initialize();
			void InitializeMqtt();	
			void Enable();
			void Disable();
			void EnableMqtt();
			void DisableMqtt();
#warning move MQTT to own class when implemented.
		}
	}
}
#endif
#endif

