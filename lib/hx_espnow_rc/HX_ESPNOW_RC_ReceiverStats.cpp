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
    
    this->lastReceivedTimeMs = t - FAILSAFE_PERIOD_MS;

    this->prevSequenceId = 0;
    this->packetsSuccess = 0;
    this->packetsError = 0;

    this->RSSIUpdateMs = t;
    this->RSSIPacketsSuccess = 0;
    this->RSSIPacketsError = 0;
    this->RSSILast = 0;

    this->remoteReceiverRSSI = -1;
    this->telemetryBytesReceivedTotal = 0;
    this->packetsRetransmit = 0;
    this->packetsCRCError = 0;
    this->packetsInvalidLength = 0;

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
    return delta >= FAILSAFE_PERIOD_MS;
}

//=====================================================================
//=====================================================================
//(percentage of packets received) 0...100
//or -1 if not received for a long time
uint8_t HXRCReceiverStats::getRSSI()
{
    unsigned long t = millis();
    unsigned long delta = t - this->RSSIUpdateMs;

    if ( delta > 1000)
    {
        uint16_t packetsSuccessCount = this->packetsSuccess - this->RSSIPacketsSuccess;
        uint16_t packetsErrorCount = this->packetsError - this->RSSIPacketsError;
        uint16_t totalPackets = packetsSuccessCount + packetsErrorCount;

        this->RSSILast = ( totalPackets > 0 ) ? (((uint32_t)packetsSuccessCount) * 100 / totalPackets) : 0;
        this->RSSIPacketsSuccess = this->packetsSuccess;
        this->RSSIPacketsError = this->packetsError;
        this->RSSIUpdateMs = t; 
    }
    return this->RSSILast;


}

//=====================================================================
//=====================================================================
int8_t HXRCReceiverStats::getRemoteReceiverRSSI()
{
    if ( isFailsafe() )
    {
        return -1;
    }

    return this->remoteReceiverRSSI;
}


//=====================================================================
//=====================================================================
bool HXRCReceiverStats::onPacketReceived( uint16_t sequenceId, int8_t rssi, uint8_t telemetrySize )
{
    uint16_t seqDelta = sequenceId - this->prevSequenceId;

    if ( seqDelta == 0 )
    {
        this->packetsRetransmit++;
        return false;
    }

    this->remoteReceiverRSSI = rssi;
    this->telemetryBytesReceivedTotal += telemetrySize;

    if ( (seqDelta > 1) && ( this->prevSequenceId!=0 ) )
    {
        this->packetsError += seqDelta - 1;
    }
    
    this->packetsSuccess++;

    this->prevSequenceId = sequenceId;
    this->lastReceivedTimeMs = millis();

    return true;
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
    Serial.printf("Receiver stats: -----------\n");
    Serial.printf("Failsafe: %d\n", isFailsafe()?1:0);
    Serial.printf("RSSI: %d\n", getRSSI() );
    Serial.printf("Remote receiver RSSI: %i\n", getRemoteReceiverRSSI() );
    Serial.printf("Packets received(retransmitted): %u(%u)\n", packetsSuccess, packetsRetransmit);
    Serial.printf("Packets missed: %u\n", packetsError);
    Serial.printf("Packets invalid length/crc: %u/%u\n", packetsInvalidLength, packetsCRCError);
    Serial.printf("Telemetry overflow count: %u\n", telemetryOverflowCount);
    Serial.printf("In telemetry: %d b/s\n", getTelemetryReceivedSpeed());
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::onTelemetryOverflow()
{
    this->telemetryOverflowCount++;  
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::onInvalidLengthPacket()
{
    this->packetsInvalidLength++;
}

//=====================================================================
//=====================================================================
void HXRCReceiverStats::onPacketCRCError()
{
    this->packetsCRCError++;
}
