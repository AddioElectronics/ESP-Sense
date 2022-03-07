#ifndef _WEBCONSOLE_h
#define _WEBCONSOLE_h

#include <Arduino.h>
#include "../../Config/config_master.h"

#include <ESPAsyncWebServer.h>

namespace Network {
	namespace Website {
		namespace Console {
			void Initialize();
			void Enable();
			void Disable();
		}

	}
}

#endif

