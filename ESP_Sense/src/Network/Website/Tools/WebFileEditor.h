#ifndef _WEBFILEEDITOR_h
#define _WEBFILEEDITOR_h

#include <Arduino.h>
#include "../../../Config/config_master.h"
#include "../../../Config/global_status.h"

#include <ESPAsyncWebServer.h>


#if COMPILE_FILE_EDITOR
namespace Network {
	namespace Website {
		namespace Tools {
			namespace FileEditor {
				void Initialize();
				void Enable();
				void Disable();

			}
		}
	}
}
#endif

#endif

