#ifndef _MQTTTOPICMANAGER_h
#define _MQTTTOPICMANAGER_h

#include <Arduino.h>

//#include <List.hpp>

/*
As storing topics for every device and parent uses a lot of ram, and most of them share the same text,
this will store the strings, not keeping duplicated, and topics will be built as they are needed.

This is a WIP, and custom topics have been removed until this is finished.
*/

namespace Mqtt {
	namespace TopicManager {

		/*List<String> strings;

		int FindString(String& s) {
			for (int i = 0; i < strings.getSize(); i++)
			{
				if (*strings[i] == s)
					return i;
			}
			return -1;
		}

		int FindString(const char* s) {
			for (int i = 0; i < strings.getSize(); i++)
			{
				if (*strings[i] == s)
					return i;
			}
			return -1;
		}

		int AddString(String& s) {
			int index = -1;

			index = FindString(s);

			if(index == -1)
			{
				index = strings.getSize();
				strings.add(s);
			}

			return index;
		}

		int AddString(const char* s) {
			int index = -1;

			index = FindString(s);

			if (index == -1)
			{
				String str(s);
				index = strings.getSize();
				strings.add(str);
			}

			return index;
		}
		*/



	}
}

#endif

