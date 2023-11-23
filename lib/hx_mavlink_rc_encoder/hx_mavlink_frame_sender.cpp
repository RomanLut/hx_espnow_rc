#include "hx_mavlink_frame_sender.h"
#include <common/mavlink.h>

#define MAX_CHUNK_SIZE 253 //size of ENCAPSULATED_DATA payload

#define MAX_FRAME_SIZE (MAX_CHUNK_SIZE*900)  //~227Kb

//=====================================================================
//=====================================================================
HXMavlinkFrameSender::HXMavlinkFrameSender()
{
    this->frameData = NULL;
    this->state = FS_IDLE;
    this->hasNewFrameData = false;

    this->frameDataMutex = xSemaphoreCreateMutex();
}

//=====================================================================
//=====================================================================
HXMavlinkFrameSender::~HXMavlinkFrameSender()
{
    if (!this->frameData) free(this->frameData);
    vSemaphoreDelete( this->frameDataMutex );
}

//=====================================================================
//=====================================================================
bool HXMavlinkFrameSender::send(HXRCSerialBufferBase& buffer)
{
    //Serial.print("framedata=");
    //Serial.print((int)(this->frameData));

    xSemaphoreTake(this->frameDataMutex, portMAX_DELAY); 

    if ( this->state == FS_IDLE ) 
    {
        if ( this->hasNewFrameData )
        {
            this->state = FS_FRAME_HEADER;
        }
        else
        {
            xSemaphoreGive(this->frameDataMutex); 
            return false; //nothing to send
        }
    }

    if ( this->state == FS_FRAME_HEADER )
    {
        if (!this->trySendFrameHeader(buffer)) 
        { 
            xSemaphoreGive(this->frameDataMutex); 
            return false;
        }
    }

    while ( true )
    {
        if ( this->state == FS_CHUNK )
        {
            if  (!this->trySendChunk(buffer)) 
            {
                xSemaphoreGive(this->frameDataMutex); 
                return false;
            }
        }
        else
        {
            //state == FS_IDLE
            xSemaphoreGive(this->frameDataMutex); 
            return true; //should add frame and call again
        }
    }
    xSemaphoreGive(this->frameDataMutex); 
}

//=====================================================================
//=====================================================================
bool HXMavlinkFrameSender::isEmpty()
{
    return this->state == FS_IDLE;
}

//=====================================================================
//=====================================================================
void HXMavlinkFrameSender::addFrame(const uint8_t* frame, int size)
{
    if ( size > MAX_FRAME_SIZE ) return;

    xSemaphoreTake(this->frameDataMutex, portMAX_DELAY); 

    if ( this->state != FS_IDLE ) 
    {
        xSemaphoreGive(this->frameDataMutex); 
        return;  //do not accept new frame until current one is not sent completely
    }

    //allocate here not in constructor, so frameData could possibly go into PSRAM
    //PSRAM is not availabe in static constructors
    if (!this->frameData)
    {
        this->frameData = (uint8_t*)ps_malloc( MAX_FRAME_SIZE );
    }

    this->hasNewFrameData = true;
    memcpy( this->frameData, frame, size);
    this->frameSize = size;

    xSemaphoreGive(this->frameDataMutex); 
}

uint8_t binc = 0;

//=====================================================================
//=====================================================================
bool HXMavlinkFrameSender::trySendFrameHeader(HXRCSerialBufferBase& buffer)
{
    mavlink_message_t msg;

    mavlink_msg_data_transmission_handshake_pack(255, MAV_COMP_ID_CAMERA, &msg, 
            0, //type
            this->frameSize,  //Size
            800, //width
            600, //height
            0, //packets,
            MAX_CHUNK_SIZE, //payload,
            85 //jpg_quality
    );

/*
    mavlink_get_channel_status(MAVLINK_COMM_0)->flags = MAVLINK_STATUS_FLAG_OUT_MAVLINK1;

    mavlink_finalize_message_chan(
        &msg, 1, MAV_COMP_ID_CAMERA,
        MAVLINK_COMM_0, 
        MAVLINK_MSG_ID_DATA_TRANSMISSION_HANDSHAKE_MIN_LEN, 
        MAVLINK_MSG_ID_DATA_TRANSMISSION_HANDSHAKE_LEN, 
        MAVLINK_MSG_ID_DATA_TRANSMISSION_HANDSHAKE_CRC);
*/

    uint8_t sbuf[MAVLINK_MAX_PACKET_LEN];  
    int len = mavlink_msg_to_send_buffer(sbuf, &msg); 

    if (buffer.getAvailableForWrite() < len ) return false;

    //strcpy((char*)sbuf, "HEADER\n");
    //len = strlen((char*)sbuf);

/*
//link test
for ( int i = 0; i< len; i++ ) sbuf[i] = binc++;
*/

    buffer.writeBuffer( sbuf, len );

    this->state = FS_CHUNK;
    this->chunk = 0;

    return true;
}

//=====================================================================
//=====================================================================
bool HXMavlinkFrameSender::trySendChunk(HXRCSerialBufferBase& buffer)
{
    mavlink_message_t msg;

    int thisChunkSize = this->frameSize - this->chunk * MAX_CHUNK_SIZE;
    bool lastChunk = thisChunkSize <= MAX_CHUNK_SIZE;
    if ( !lastChunk) thisChunkSize = MAX_CHUNK_SIZE;

    mavlink_msg_encapsulated_data_pack(255, MAV_COMP_ID_CAMERA, &msg, this->chunk, &(this->frameData[this->chunk * MAX_CHUNK_SIZE]));

/*
    mavlink_get_channel_status(MAVLINK_COMM_0)->flags = MAVLINK_STATUS_FLAG_OUT_MAVLINK1;

    mavlink_finalize_message_chan(
        &msg, 1, MAV_COMP_ID_CAMERA,
        MAVLINK_COMM_0, 
        MAVLINK_MSG_ID_ENCAPSULATED_DATA_MIN_LEN, 
        MAVLINK_MSG_ID_ENCAPSULATED_DATA_LEN, 
        MAVLINK_MSG_ID_ENCAPSULATED_DATA_CRC);
*/
    uint8_t sbuf[MAVLINK_MAX_PACKET_LEN];  
    int len = mavlink_msg_to_send_buffer(sbuf, &msg); //len = 264(v1) or 279(v2)  

    int avail = buffer.getAvailableForWrite();
    if (avail < len ) return false;

    //strcpy((char*)sbuf, "RED FOX JUMPS OVER THE LAZY DOG\n");
    //len = strlen((char*)sbuf);

/*
//link test
for ( int i = 0; i< len; i++ ) sbuf[i] = binc++;
*/

    buffer.writeBuffer( sbuf, len );

    if ( lastChunk)
    {
        this->state = FS_IDLE;
        this->hasNewFrameData = false;
    }
    else
    {
        this->state = FS_CHUNK;
        this->chunk++;
    }
    
    return true;
}

//=====================================================================
//=====================================================================
void HXMavlinkFrameSender::dumpState()
{
    switch ( this->state )
    {
        case FS_IDLE:
            Serial.print("IDLE\n");
        break;

        case FS_FRAME_HEADER:
            Serial.print("HEADER\n");
        break;

        case FS_CHUNK:
            Serial.print("CHUNK:");
            Serial.print(this->chunk);
            Serial.print("\n");
        break;
    }
}
