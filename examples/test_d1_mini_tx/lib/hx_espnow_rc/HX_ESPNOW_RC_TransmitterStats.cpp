#include "HX_ESPNOW_RC_TransmitterStats.h"

//=====================================================================
//=====================================================================
HXRCTransmitterStats::HXRCTransmitterStats()
{
    reset();
}

//=====================================================================
//=====================================================================
void HXRCTransmitterStats::reset()
{
    unsigned long t = millis();

    this->packetsSentTotal = 0;
    this->packetsSentSuccess = 0;
    this->packetsSentError = 0;
    this->packetsNotSentInTime = 0;

    this->lastSendTimeMs = t;
    this->lastSuccessfulPacketMs = t - FAILSAFE_PERIOD_MS;

    this->RSSIPacketsSentSuccess = 0;
    this->RSSIPacketsSentError = 0;
    this->RSSIupdateMs = t; 
    this->RSSIlast = 0;

    this->telemetryBytesSentTotal = 0;

    this->lastTelemetryBytesSentSpeed = 0;
    this->lastTelemetryBytesSentTotal = 0;
    this->telemetrySpeedUpdateMs = t;
}

//=====================================================================
//=====================================================================
bool HXRCTransmitterStats::isFailsafe()    
{
    unsigned long delta = millis() - this->lastSuccessfulPacketMs;
    return delta >= FAILSAFE_PERIOD_MS;
}

//=====================================================================
//=====================================================================
//(percentage of packets sent with aknowledgement) 0...100
uint8_t HXRCTransmitterStats::getRSSI()
{
    unsigned long t = millis();
    unsigned long delta = t - this->RSSIupdateMs;

    if ( delta > 1000)
    {
        uint16_t packetsSuccessCount = this->packetsSentSuccess - this->RSSIPacketsSentSuccess;
        uint16_t packetsErrorCount = this->packetsSentError + this->packetsNotSentInTime - this->RSSIPacketsSentError;
        uint16_t totalPackets = packetsSuccessCount + packetsErrorCount;

        this->RSSIlast = ( totalPackets > 0 ) ? (((uint32_t)packetsSuccessCount) * 100 / totalPackets) : 0;
        this->RSSIPacketsSentSuccess = this->packetsSentSuccess;
        this->RSSIPacketsSentError = this->packetsSentError + this->packetsNotSentInTime;
        this->RSSIupdateMs = t; 
    }
    return this->RSSIlast;
}

//=====================================================================
//=====================================================================
void HXRCTransmitterStats::onPacketSendSuccess( uint8_t telemetryLength )
{
    this->packetsSentSuccess++;
    this->telemetryBytesSentTotal += telemetryLength;
    this->lastSuccessfulPacketMs = millis();
}

//=====================================================================
//=====================================================================
void HXRCTransmitterStats::onPacketSendError()
{
    this->packetsSentError++;
}

//=====================================================================
//=====================================================================
void HXRCTransmitterStats::onPacketSend( unsigned long timeMs )
{
    this->lastSendTimeMs = timeMs;
    this->packetsSentTotal++;
}

//=====================================================================
//=====================================================================
void HXRCTransmitterStats::onPacketSendMiss( uint16_t missedPackets )
{
    this->packetsNotSentInTime += missedPackets;
}

//telemetry send speed stats, bytes/sec
uint32_t HXRCTransmitterStats::getTelemetrySendSpeed()
{
    unsigned long t = millis();
    unsigned long delta = t - this->telemetrySpeedUpdateMs;

    if ( delta > 1000 )
    {
        this->lastTelemetryBytesSentSpeed = ((uint32_t)(this->telemetryBytesSentTotal - this->lastTelemetryBytesSentTotal))*1000 / delta;
        this->lastTelemetryBytesSentTotal = this->telemetryBytesSentTotal;
        this->telemetrySpeedUpdateMs = t;
    }

    return this->lastTelemetryBytesSentSpeed;
}

//=====================================================================
//=====================================================================
void HXRCTransmitterStats::update()
{
    getTelemetrySendSpeed(); 
    getRSSI(); 
}

//=====================================================================
//=====================================================================
void HXRCTransmitterStats::printStats()
{
    Serial.printf("Transmitter stats: -----------\n");
    Serial.printf("Failsafe: %d\n", isFailsafe()?1:0);
    Serial.printf("RSSI: %d\n", getRSSI() );
    Serial.printf("Packets sent total: %u\n", packetsSentTotal);
    Serial.printf("Packets success: %u\n", packetsSentSuccess);
    Serial.printf("Packets error: %u\n", packetsSentError);
    Serial.printf("Packets missed: %u\n", packetsNotSentInTime);
    Serial.printf("Out telemetry: %u b/s\n", getTelemetrySendSpeed());
}

