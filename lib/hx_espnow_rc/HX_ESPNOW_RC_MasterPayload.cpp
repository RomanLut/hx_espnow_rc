#include "HX_ESPNOW_RC_MasterPayload.h"
#include "HX_ESPNOW_RC_Config.h"

//=====================================================================
//=====================================================================
void HXRCMasterPayload::setCRC()
{
    uint32_t c = HXRC_crc32( (uint8_t*)&this->key, HXRC_MASTER_PAYLOAD_SIZE_BASE + this->length - 4);
    uint8_t v = HXRC_PROTOCOL_VERSION;
    this->crc = HXRC_crc32( (uint8_t*)&v, 1, c );
}

//=====================================================================
//=====================================================================
bool HXRCMasterPayload::checkCRC() const
{
    uint32_t c = HXRC_crc32( (uint8_t*)&this->key, HXRC_MASTER_PAYLOAD_SIZE_BASE + this->length - 4);
    uint8_t v = HXRC_PROTOCOL_VERSION;
    c = HXRC_crc32( (uint8_t*)&v, 1, c );
    return c == this->crc;
}

