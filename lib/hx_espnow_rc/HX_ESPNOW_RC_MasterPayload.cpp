#include "HX_ESPNOW_RC_MasterPayload.h"
#include "HX_ESPNOW_RC_Config.h"

//=====================================================================
//=====================================================================
void HXRCMasterPayload::setCRC( HXRCConfig& config )
{
    this->crc = config.keyCRC;
    HXRC_crc16_update_buffer( &this->crc, (uint8_t*)&this->sequenceId, HXRC_MASTER_PAYLOAD_SIZE_BASE + this->length - 2);
}

//=====================================================================
//=====================================================================
bool HXRCMasterPayload::checkCRC( HXRCConfig& config ) const
{
    uint16_t crc = config.keyCRC;
    HXRC_crc16_update_buffer( &crc, (uint8_t*)&this->sequenceId, HXRC_MASTER_PAYLOAD_SIZE_BASE + this->length - 2);
    return crc == this->crc;
}

