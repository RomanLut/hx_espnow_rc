#include "dvr_config.h"

int DVRQuality = 1;
TDVRQualitySettings DVRQualitySettings[3] = 
{
    {
        frameSize:          DVR_LQ_FRAMESIZE,
        frameSize16x9mod:   DVR_LQ_FRAMESIZE_16x9,
        fps:                DVR_LQ_FPS,
        jpegQuality:        DVR_LQ_JPEG_QUALITY,
        sharpness:          DVR_LQ_SHARPNESS,
        saturation:         DVR_LQ_SATURATION
    },
    {
        frameSize:          DVR_MQ_FRAMESIZE,
        frameSize16x9mod:   DVR_MQ_FRAMESIZE_16x9,
        fps:                DVR_MQ_FPS,
        jpegQuality:        DVR_MQ_JPEG_QUALITY,
        sharpness:          DVR_MQ_SHARPNESS,
        saturation:         DVR_MQ_SATURATION
    },
    {
        frameSize:          DVR_HQ_FRAMESIZE,
        frameSize16x9mod:   DVR_HQ_FRAMESIZE_16x9,
        fps:                DVR_HQ_FPS,
        jpegQuality:        DVR_HQ_JPEG_QUALITY,
        sharpness:          DVR_HQ_SHARPNESS,
        saturation:         DVR_HQ_SATURATION
    },
};