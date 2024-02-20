/*
 * VideoDecoder.h
 *
 * ffmpeg video decoder with opencv viewing and saving support
 * NOTE: this decoder can decode only one AVFrame per AVPacket (i.e., per decode(...) call)
 *
 * Created on: May 11, 2022
 *      Author: Nikita Smirnov
 */

#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class VideoDecoder {
    // hardcoded because it is played/saved further by OpenCV so far
    static const AVPixelFormat PIXEL_FORMAT = AV_PIX_FMT_BGR24;
public:
    VideoDecoder(int verbose = 0);
    virtual ~VideoDecoder();

    // main initialization
    void init(AVCodecID codecId = AV_CODEC_ID_H264);

    // main decode call
    bool decode(uint8_t* encodedBuffer, size_t size);

    // opencv viewer
    void initViewer(const std::string& windowName = "videoDecoder");
    void view();

    // opencv writer
    void initWriter(const std::string& filename = "videoOutput.mp4",
                    int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
                    int fps = 25,
                    cv::Size size = cv::Size(1920, 1080));
    void write();

    cv::Mat& getOutputFrame();
    int getFramesReceived();
    int getFramesDecoded();

private:
    bool isEncodingChanged();
    void flush();

public:
    bool isCvMatReady;
    bool isPlaying;
    int verbose;

private:
    // ffmpeg placeholders
    AVCodec* codec_;
    AVCodecContext* codecContext_;
    AVFrame* rawFrame_;
    AVFrame* convFrame_;
    AVPacket* packet_;
    struct SwsContext* sws_;

    // containers for image scaling
    uint8_t* srcBuffer_;
    int srcSize_;
    int srcWidth_;
    int srcHeight_;

    // opencv
    cv::Mat outputFrame_;
    std::string windowName_;

    cv::VideoWriter videoWriter_;
    cv::Mat vwFrame_;
    cv::Size vwFrameSize_;

    // stats
    int framesReceived_;
    int framesDecoded_;
};

#endif