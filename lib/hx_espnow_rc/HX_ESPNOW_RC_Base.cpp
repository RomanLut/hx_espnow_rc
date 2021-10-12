#include "HX_ESPNOW_RC_Base.h"

//=====================================================================
//=====================================================================
HXRCBase::HXRCBase()
{
    HXRC_crc32_init();
    senderState = HXRCSS_INIT;
    receivedSequenceId = 0xffff;    
    waitAck = false;
    memset(peerMac,0,6);
}

//=====================================================================
//=====================================================================
HXRCBase::~HXRCBase()
{
}

//=====================================================================
//=====================================================================
bool HXRCBase::init( HXRCConfig config )
{
    this->config = config;

    this->transmitterStats.reset();
    this->receiverStats.reset();

    HXRCInitLedPin(config);

    this->A1 = 0;
    this->A2 = 0;

    senderState = HXRCSS_READY_TO_SEND;

    return true;
}

//=====================================================================
//=====================================================================
void HXRCBase::loop()
{
    transmitterStats.update();
    receiverStats.update();

    updateLed( this->config.ledPin, this->config.ledPinInverted);
}

//=====================================================================
//=====================================================================
HXRCTransmitterStats& HXRCBase::getTransmitterStats() 
{
    return this->transmitterStats;
}

//=====================================================================
//=====================================================================
HXRCReceiverStats& HXRCBase::getReceiverStats() 
{
    return this->receiverStats;
}

//=====================================================================
//=====================================================================
void HXRCBase::updateLed( int8_t ledPin, bool ledPinInverted )
{
    if ( ledPin == -1) return;

    if ( getTransmitterStats().isFailsafe())
    {
        digitalWrite(ledPin, ledPinInverted ? HIGH : LOW );
    }
    else
    {
        unsigned long dt = millis() - this->receiverStats.lastReceivedTimeMs;
        if ( dt < 300 )
        {
            digitalWrite(ledPin,(millis() & 32) ? HIGH : LOW );
        }
        else
        {
            digitalWrite(ledPin, ledPinInverted ? LOW : HIGH );
        }
    }
}

//=====================================================================
//=====================================================================
uint16_t HXRCBase::getIncomingTelemetry(uint16_t maxSize, uint8_t* pBuffer)
{
    return this->incomingTelemetryBuffer.receiveUpTo( maxSize, pBuffer);
}

//=====================================================================
//=====================================================================
bool HXRCBase::sendOutgoingTelemetry( uint8_t* ptr, uint16_t size )
{
    return this->outgoingTelemetryBuffer.send( ptr, size );
}

//=====================================================================
//=====================================================================
const uint8_t* HXRCBase::getPeerMac() const
{
    return this->peerMac;
}

