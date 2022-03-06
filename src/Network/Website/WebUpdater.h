#ifndef _WEBUPDATER_h
#define _WEBUPDATER_h

#include <Arduino.h>
#include "../../Config/config_master.h"

#include <ESPAsyncWebServer.h>

namespace Network {
	namespace Website {
		namespace WebUpdate {
			void Initialize();
			void Enable();
			void Disable();			
		}

	}
}

#endif

