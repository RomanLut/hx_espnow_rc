#include "HX_ESPNOW_RC_ReceiverStats.h"


//=====================================================================
//=====================================================================
HXRCReceiverStats::HXRCReceiverStats()
{
    reset();
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::reset()
{
    unsigned long t = millis();
    
    this->lastReceivedTimeMs = t - DEFAULT_FAILSAFE_PERIOD_MS;

    this->prevPacketId = 0xffff;
    this->prevSequenceId = 0xffff;
    this->packetsReceived = 0;
    this->packetsLost = 0;

    this->RSSIUpdateMs = t;
    this->RSSIPacketsReceived = 0;
    this->RSSIPacketsLost = 0;
    this->RSSILast4 = 0;

    this->telemetryBytesReceivedTotal = 0;
    this->packetsRetransmit = 0;
    this->packetsCRCError = 0;
    this->packetsInvalid = 0;

    this->lastTelemetryBytesReceivedSpeed = 0;
    this->lastTelemetryBytesReceivedTotal = 0;
    this->telemetrySpeedUpdateMs = t;

    this->telemetryOverflowCount = 0;
}

//=====================================================================
//=====================================================================
bool HXRCReceiverStats::isFailsafe()    
{
    unsigned long delta = millis() - this->lastReceivedTimeMs;
    return delta >= DEFAULT_FAILSAFE_PERIOD_MS;
}

//=====================================================================
//=====================================================================
//(percentage of packets received) 0...100
uint8_t HXRCReceiverStats::getRSSI()
{
    unsigned long t = millis();
    unsigned long delta = t - this->RSSIUpdateMs;

    if ( delta > 1000)
    {
        uint16_t packetsSuccessCount = this->packetsReceived - this->RSSIPacketsReceived;
        uint16_t packetsLostCount = this->packetsLost - this->RSSIPacketsLost;
        uint16_t packetsTotalCount = packetsSuccessCount + packetsLostCount;

        this->RSSILast4 -= this->RSSILast4 >> 2;
        this->RSSILast4 += ( packetsTotalCount > 0 ) ? (((uint32_t)packetsSuccessCount) * 100 / packetsTotalCount) : 0;
        this->RSSIPacketsReceived = this->packetsReceived;
        this->RSSIPacketsLost = this->packetsLost;
        this->RSSIUpdateMs = t; 
    }
    return this->RSSILast4 >> 2;
}

//=====================================================================
//=====================================================================
uint8_t HXRCReceiverStats::getRemoteRSSIDbm()
{
    return this->remoteRSSIDbm;
}

//=====================================================================
//=====================================================================
uint8_t HXRCReceiverStats::getRemoteNoiseFloor()
{
    return this->remoteNoiseFloor;
}

//=====================================================================
//=====================================================================
uint8_t HXRCReceiverStats::getRemoteSNR()
{
    return this->remoteNoiseFloor - this->remoteRSSIDbm;
}

//=====================================================================
//=====================================================================
bool HXRCReceiverStats::onPacketReceived( uint16_t packetId, uint16_t sequenceId, uint8_t telemetrySize, uint8_t RSSIDbm, uint8_t noiseFloor )
{
    this->packetsReceived++;

    bool res = sequenceId != this->prevSequenceId;
    if ( res )
    {
        this->telemetryBytesReceivedTotal += telemetrySize;
    }
    else
    {
        this->packetsRetransmit++;
    }

    uint16_t delta = packetId - this->prevPacketId;
    if ( delta > 1 )
    {
        this->packetsLost += delta-1;
    }

    this->prevPacketId = packetId;
    this->prevSequenceId = sequenceId;
    this->lastReceivedTimeMs = millis();

    this->remoteRSSIDbm = RSSIDbm;
    this->remoteNoiseFloor = noiseFloor;

    return res;
}

//=====================================================================
//=====================================================================
//telemetry receive speed stats, bytes/sec
uint32_t HXRCReceiverStats::getTelemetryReceivedSpeed()
{
    unsigned long t = millis();
    uint32_t delta = t - this->telemetrySpeedUpdateMs;

    if ( delta > 1000 )
    {
        this->lastTelemetryBytesReceivedSpeed = ((uint32_t)(this->telemetryBytesReceivedTotal - this->lastTelemetryBytesReceivedTotal)) * 1000 / delta;
        this->lastTelemetryBytesReceivedTotal = this->telemetryBytesReceivedTotal;
        this->telemetrySpeedUpdateMs = t;
    }

    return this->lastTelemetryBytesReceivedSpeed;
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::update()
{
    getTelemetryReceivedSpeed(); 
    getRSSI(); 
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::printStats()
{
    HXRCLOG.printf("Receiver   ");
    HXRCLOG.printf(" | FS: %d", isFailsafe()?1:0);
    HXRCLOG.printf(" | RSSI: %d", getRSSI() );
    HXRCLOG.printf(" | Recv(retransm): %u(%u)", packetsReceived, packetsRetransmit);
    HXRCLOG.printf(" | Lost: %u", packetsLost);
    HXRCLOG.printf(" | Invalid/CRC: %u/%u", packetsInvalid, packetsCRCError);
    HXRCLOG.printf(" | Tel. overflow: %u", telemetryOverflowCount);
    HXRCLOG.printf(" | In telemetry: %d b/s\n", getTelemetryReceivedSpeed());
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::onTelemetryOverflow()
{
    this->telemetryOverflowCount++;  
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::onInvalidPacket()
{
    this->packetsInvalid++;
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::onPacketCRCError()
{
    this->packetsCRCError++;
}
