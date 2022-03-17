
#ifndef _WEBSITEMANAGER_h
#define _WEBSITEMANAGER_h

#include <Arduino.h>

#include "../../Config/config_master.h"

#include "../../Config/global_status.h"
#if COMPILE_SERVER

namespace Network {
	namespace Website {
		void Initialize();
		void StopHosting();
	}
}

#endif
#endif

