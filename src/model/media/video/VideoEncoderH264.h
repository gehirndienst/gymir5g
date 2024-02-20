/*
 * VideoEncoderH264.h
 *
 * x264 encoder for video with controlled frame size, fps and bitrate (VBV)
 *
 * Created on: May 7, 2022
 *      Author: Nikita Smirnov
 */

#ifndef H264_ENCODER_H
#define H264_ENCODER_H

#include <stdint.h>
#include <iostream>
#include <vector>

extern "C" {
#include <x264.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
}

#include "model/common/QualityOfContent.h"
#include "model/presets/VideoQualityPresets.h"

class VideoEncoderH264 {
    // so far hardcoded because raw data is captured by OpenCV
    static const AVPixelFormat PIXEL_FORMAT = AV_PIX_FMT_BGR24;
    // so far fix to 25
    static const int FPS = 25;
    // max UDP datagram compliant
    inline static const int X264_MAX_NAL_SIZE = 64000;
public:
    VideoEncoderH264(int maxPacketSize = 64000, int verbose = 0);
    virtual ~VideoEncoderH264();

    void init(VideoQuality quality, VideoEncodingSpeed speed, int srcWidth, int srcHeight);
    bool changeBitrate(int bitrateKbps);

    bool encode(uint8_t* rawImgBuffer);

    void getNals(uint8_t* nalsBuffer);
    std::vector<std::pair<uint8_t*, int>>& getNals();
    QualityOfVideo& getQov();
    int getFrameSize();
    int getFramesEncoded();
    bool isKeyFrame();

    void flush();

public:
    int maxPacketSize;
    bool isOk;
    int verbose;

private:
    int srcWidth_;
    int srcHeight_;
    int dstWidth_;
    int dstHeight_;

    // x264-specific
    x264_t* enc_;
    x264_param_t prms_;
    x264_picture_t picIn_, picOut_;
    x264_nal_t* nals_;
    int nheader_;
    int nalsNum_;

    // ffmpeg
    AVFrame* picRaw_;
    struct SwsContext* sws_;

    // placeholders
    std::vector<std::pair<uint8_t*, int>> encodedNals_;
    QualityOfVideo qov_;
    bool isKeyFrame_;
    int framesEncoded_;
    int frameSize_;
};

#endif /* H264_ENCODER_H */
