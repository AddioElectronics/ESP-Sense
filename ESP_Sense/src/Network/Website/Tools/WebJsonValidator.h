#ifndef _WEBJSONVALIDATOR_h
#define _WEBJSONVALIDATOR_h

#include <Arduino.h>
#include "../../../Config/config_master.h"
#include "../../../Config/global_status.h"

#include <ESPAsyncWebServer.h>


#if COMPILE_WEB_JSON_VALIDATOR
namespace Network {
	namespace Website {
		namespace Tools {
			namespace JsonValidator {
				void Initialize();
				void Enable();
				void Disable();

			}
		}
	}
}
#endif

#endif

