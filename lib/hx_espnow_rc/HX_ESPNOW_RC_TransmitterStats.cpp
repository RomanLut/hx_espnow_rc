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
    this->packetsAcknowledged = 0;  //acknowledged packets
    this->packetsSentError = 0;
    this->packetsNotSentInTime = 0;

    this->lastSendTimeMs = t;
    this->lastAcknowledgedPacketMs = t - DEFAULT_FAILSAFE_PERIOD_MS;

    this->RSSIPacketsAcknowledged = 0;
    this->RSSIPacketsTotal = 0;
    this->RSSIUpdateMs = t; 
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
    unsigned long delta = millis() - this->lastAcknowledgedPacketMs;
    return delta >= DEFAULT_FAILSAFE_PERIOD_MS;
}

//=====================================================================
//=====================================================================
//(percentage of packets sent with aknowledgement) 0...100
uint8_t HXRCTransmitterStats::getRSSI()
{
    unsigned long t = millis();
    unsigned long delta = t - this->RSSIUpdateMs;

    if ( delta > 1000)
    {
        uint16_t packetsSuccessCount = this->packetsAcknowledged - this->RSSIPacketsAcknowledged;
        this->successfullPacketRateLast = packetsSuccessCount;
        uint16_t packetsTotalCount = this->packetsSentTotal + this->packetsNotSentInTime - this->RSSIPacketsTotal; 

        this->RSSIlast = ( packetsTotalCount > 0 ) ? (((uint32_t)packetsSuccessCount) * 100 / packetsTotalCount) : 0;
        
        this->RSSIPacketsAcknowledged = this->packetsAcknowledged;
        this->RSSIPacketsTotal = this->packetsSentTotal + this->packetsNotSentInTime;
        
        this->RSSIUpdateMs = t; 
    }
    return this->RSSIlast;
}

//=====================================================================
//=====================================================================
uint8_t HXRCTransmitterStats::getSuccessfulPacketRate()
{
    return this->successfullPacketRateLast;
}

//=====================================================================
//=====================================================================
uint8_t HXRCTransmitterStats::getRSSIDbm()
{
#if defined (ESP32)    
    return -capture.rssi;
#else 
    return 0;
#endif    
}

//=====================================================================
//=====================================================================
uint8_t HXRCTransmitterStats::getNoiseFloor()
{
#if defined (ESP32)    
    return -capture.noiseFloor;
#else 
    return 0;
#endif    
}

//=====================================================================
//=====================================================================
uint8_t HXRCTransmitterStats::getSNR()
{
#if defined (ESP32)    
    return (-capture.noiseFloor) - (-capture.rssi);
#else 
    return 0;
#endif    
}

//=====================================================================
//=====================================================================
int HXRCTransmitterStats::getRate()
{
#if defined (ESP32)    
    return capture.rate;
#else 
    return -1;
#endif    
}

//=====================================================================
//=====================================================================
void HXRCTransmitterStats::onPacketAck( uint8_t telemetryLength )
{
    this->packetsAcknowledged++;
    this->telemetryBytesSentTotal += telemetryLength;
    this->lastAcknowledgedPacketMs = millis();
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

//=====================================================================
//=====================================================================
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
    HXRCLOG.print("Transmitter");
    HXRCLOG.printf(" | FS: %d", isFailsafe()?1:0);
    HXRCLOG.printf(" | RSSI: %d", getRSSI() );
    HXRCLOG.printf(" | Total: %u", packetsSentTotal);
    HXRCLOG.printf(" | Ack: %u", packetsAcknowledged);
    HXRCLOG.printf(" | Error: %u", packetsSentError);
    HXRCLOG.printf(" | Missed time: %u", packetsNotSentInTime);
    HXRCLOG.printf(" | PacketRate: %dp/s", getSuccessfulPacketRate());
    HXRCLOG.printf(" | Out telemetry: %u b/s\n", getTelemetrySendSpeed());
#if defined(ESP32)
    HXRCLOG.printf(" RSSIDBm: -%ddbm", getRSSIDbm());
    HXRCLOG.printf(" | Noise Floor: -%ddbm", getNoiseFloor());
    HXRCLOG.printf(" | SNR: %ddb", getSNR());
    HXRCLOG.printf(" | WifiRate: %i\n", getRate());
#endif    
}

