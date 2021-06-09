#pragma once

#include <Arduino.h>

#include "HX_ESPNOW_RC_ReceiverStats.h"
#include "HX_ESPNOW_RC_TransmitterStats.h"

//TODO: settings:
//Receiver MAC address
//channel

void HXRCInit( int channel);
void HXRCLoop();

//index = 0..15
//data = 1000...2000
void HXRCGetChannelValue( uint8_t index, uint16_t data);

HXRCReceiverStats& HXRCGetReceiverStats();
HXRCTransmitterStats& HXRCGetTransmitterStats();

