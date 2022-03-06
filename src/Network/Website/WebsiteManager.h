
#ifndef _WEBSITEMANAGER_h
#define _WEBSITEMANAGER_h

#include <Arduino.h>

#include "../../Config/config_master.h"
#if COMPILE_SERVER

namespace Network {
	namespace Website {

		void Initialize();
		void StopHosting();

		namespace Home {


			void Initialize();
			//void ResponseHome();
			//void ResponseDeviceStatus();


		}
	}
}

#endif
#endif

