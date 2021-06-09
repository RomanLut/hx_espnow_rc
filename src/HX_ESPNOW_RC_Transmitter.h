#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_Common.h"
#include "HX_ESPNOW_RC_TransmitterStats.h"
#include "HX_ESPNOW_RC_ReceiverStats.h"

//TODO: settings:
//Receiver MAC address
//channel

void HXRCInit(int wifi_channel);
void HXRCLoop();

//index = 0..15
//data = 1000...2000
void HXRCSetChannelValue( uint8_t index, uint16_t data);

HXRCTransmitterStats& HXRCGetTransmitterStats();
HXRCReceiverStats& HXRCGetReceiverStats();

