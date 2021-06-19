#include "HX_ESPNOW_RC_SlavePayload.h"
#include "HX_ESPNOW_RC_Config.h"


//=====================================================================
//=====================================================================
void HXRCSlavePayload::setCRC( HXRCConfig& config )
{
    this->crc = config.keyCRC;
    HXRC_crc16_update_buffer( &this->crc, (uint8_t*)&this->sequenceId, HXRC_SLAVE_PAYLOAD_SIZE_BASE + this->length - 2);
}

//=====================================================================
//=====================================================================
bool HXRCSlavePayload::checkCRC( HXRCConfig& config ) const
{
    uint16_t crc = config.keyCRC;
    HXRC_crc16_update_buffer( &crc, (uint8_t*)&this->sequenceId, HXRC_SLAVE_PAYLOAD_SIZE_BASE + this->length - 2);
    return crc == this->crc;
}

