/*
 * WifiEvents.h
 *
 *  Created on: 19 февр. 2016 г.
 *      Author: shurik
 */

#ifndef SMINGCORE_PLATFORM_WIFIEVENTS_H_
#define SMINGCORE_PLATFORM_WIFIEVENTS_H_

#include "../SmingCore/Delegate.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/IPAddress.h"

//Define WifiEvents Delegates types
typedef Delegate<void(String, uint8_t, uint8_t[6], uint8_t)> onStationDisconnectDelegate;
typedef Delegate<void(IPAddress, IPAddress, IPAddress)> onStationGotIPDelegate;

class WifiEventsClass
{
public:
	WifiEventsClass();
//	~WifiEvents();
	void onStationDisconnect(onStationDisconnectDelegate delegateFunction);
	void onStationGotIP(onStationGotIPDelegate delegateFunction);

private:
	onStationDisconnectDelegate onSTADisconnect = nullptr;
	onStationGotIPDelegate onSTAGotIP = nullptr;
};


extern WifiEventsClass WifiEvents;
#endif /* SMINGCORE_PLATFORM_WIFIEVENTS_H_ */
