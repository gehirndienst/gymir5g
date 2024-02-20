#include "VideoDecoder.h"

VideoDecoder::VideoDecoder(int verbose)
    : isCvMatReady(false)
    , isPlaying(false)
    , verbose(verbose)

    , codec_(nullptr)
    , codecContext_(nullptr)
    , rawFrame_(nullptr)
    , convFrame_(nullptr)
    , packet_(nullptr)
    , sws_(nullptr)

    , srcBuffer_(nullptr)
    , srcSize_(0)
    , srcWidth_(0)
    , srcHeight_(0)

    , framesReceived_(0)
    , framesDecoded_(0)
{}

VideoDecoder::~VideoDecoder() {
    flush();
}

void VideoDecoder::init(AVCodecID codecId) {
    flush();

    codec_ = const_cast<AVCodec*>(avcodec_find_decoder(codecId));
    if (!codec_) {
        std::cerr << "[VideoDecoder::VideoDecoder] couldn't find a codec for " << avcodec_get_name(codecId) << std::endl;
        return;
    }

    codecContext_ = avcodec_alloc_context3(codec_);
    if (!codecContext_) {
        std::cerr << "[VideoDecoder::VideoDecoder] couldn't allocate context for the codec" << std::endl;
        return;
    }

    if (codec_->capabilities & AV_CODEC_CAP_TRUNCATED)
        codecContext_->flags |= AV_CODEC_FLAG_TRUNCATED; /* we may send incomplete frames */
    if (codec_->capabilities & AV_CODEC_FLAG2_CHUNKS)
        codecContext_->flags2 |= AV_CODEC_FLAG2_CHUNKS;

    if (avcodec_open2(codecContext_, codec_, nullptr) < 0) {
        std::cerr << "[VideoDecoder::VideoDecoder] couldn't open the codec" << std::endl;
        return;
    }

    rawFrame_ = av_frame_alloc();
    convFrame_ = av_frame_alloc();
    if (!rawFrame_ || !convFrame_) {
        std::cerr << "[VideoDecoder::VideoDecoder] couldn't allocate frames for the codec" << std::endl;
        return;
    }

    packet_ = av_packet_alloc();
    if (!packet_) {
        std::cerr << "[VideoDecoder::VideoDecoder] couldn't allocate packet container for the codec" << std::endl;
        return;
    }

    std::cout << "VideoDecoder for codec: " << avcodec_get_name(codecId)
              << " has been succesfully initiated" << std::endl;
}

bool VideoDecoder::decode(uint8_t* buffer, size_t size) {
    if (size == 0) {
        std::cerr << "[VideoDecoder::decode] size is equal to zero: a blank frame" << std::endl;
        return false;
    }
    packet_->size = size;
    packet_->data = buffer;
    framesReceived_++;

    if (avcodec_send_packet(codecContext_, packet_) < 0) {
        isCvMatReady = false;
        if (verbose > 0) {
            std::cerr << "[VideoDecoder::decode] Decoding failed! A packet isn't sent on elem N.: " << framesReceived_
                      << std::endl;
        }
        return false;
    }

    int ret = 42;
    while (ret >= 0) {
        ret = avcodec_receive_frame(codecContext_, rawFrame_);
        if (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // NOTE: this decoder does not allow more than one frame per packet, though it is possible
            break;
        } else {
            isCvMatReady = false;
            if (verbose > 0) {
                std::cerr << "[VideoDecoder::decode] Decoding failed! A frame isn't received on elem N.: " << framesReceived_
                          << std::endl;
            }
            return false;
        }
    }

    // allocate source buffer, conversion frame and color converter if the first try or encoding has changed
    if (!srcBuffer_ || isEncodingChanged()) {
        if (srcBuffer_) {
            av_free(srcBuffer_);
        }
        srcSize_ = av_image_get_buffer_size (PIXEL_FORMAT, codecContext_->width, codecContext_->height, 1);
        srcBuffer_ = (uint8_t*)av_malloc(srcSize_);
        av_image_fill_arrays(convFrame_->data, convFrame_->linesize, srcBuffer_, PIXEL_FORMAT, codecContext_->width,
                             codecContext_->height, 1);
        sws_ = sws_getContext(
                   codecContext_->width, codecContext_->height, codecContext_->pix_fmt,
                   codecContext_->width, codecContext_->height, PIXEL_FORMAT,
                   SWS_BILINEAR, NULL, NULL, NULL
               );

        outputFrame_.create(cv::Size(codecContext_->width, codecContext_->height), CV_8UC3);
    }

    // finish decoding
    sws_scale(sws_, (const uint8_t* const*)rawFrame_->data, rawFrame_->linesize, 0, codecContext_->height, convFrame_->data,
              convFrame_->linesize);
    memcpy(outputFrame_.data, srcBuffer_, srcSize_);
    isCvMatReady = true;
    framesDecoded_++;
    return true;
}

void VideoDecoder::initViewer(const std::string& windowName) {
    if (!isPlaying) {
        windowName_ = windowName;
        cv::namedWindow(windowName_, cv::WINDOW_AUTOSIZE);
        isPlaying = true;
    }
}


void VideoDecoder::view() {
    if (isCvMatReady) {
        cv::imshow(windowName_, outputFrame_);
        if (cv::waitKey(1) == 27) {
            isPlaying = false;
            cv::destroyWindow(windowName_);
        }
    }
}

void VideoDecoder::initWriter(const std::string& filename, int fourcc, int fps, cv::Size size) {
    if (!videoWriter_.isOpened()) {
        vwFrameSize_ = size;
        if (!videoWriter_.open(filename, fourcc, fps, vwFrameSize_)) {
            std::cerr << "[VideoDecoder::initWriter] couldn't initilialize opencv video writer" << std::endl;
            return;
        }
    } else {
        if (verbose > 0) {
            std::cout << "[VideoDecoder::initWriter] re-initializing another video writer" << std::endl;
        }
        videoWriter_.release();
        initWriter(filename, fourcc, fps);
    }
}

void VideoDecoder::write() {
    // FIXME: add support for sim delays somehow
    if (isCvMatReady) {
        cv::resize(outputFrame_, vwFrame_, vwFrameSize_);
        videoWriter_.write(vwFrame_);
    }
}

cv::Mat& VideoDecoder::getOutputFrame() {
    if (!isCvMatReady) {
        outputFrame_ = cv::Mat();
    }
    return outputFrame_;
}

int VideoDecoder::getFramesReceived() {
    return framesReceived_;
}

int VideoDecoder::getFramesDecoded() {
    return framesDecoded_;
}

bool VideoDecoder::isEncodingChanged() {
    if (srcWidth_ != codecContext_->width || srcHeight_ != codecContext_->height) {
        srcWidth_ = codecContext_->width;
        srcHeight_ = codecContext_->height;
        return true;
    } else {
        return false;
    }
}

void VideoDecoder::flush() {
    if (codecContext_) {
        avcodec_close(codecContext_);
        avcodec_free_context(&codecContext_);
    }
    if (rawFrame_) {
        av_frame_free(&rawFrame_);
    }
    if (convFrame_) {
        av_frame_free(&convFrame_);
    }
    if (packet_) {
        av_packet_free(&packet_);
    }
    if (sws_) {
        sws_freeContext(sws_);
    }
    if (srcBuffer_) {
        av_free(srcBuffer_);
    }
    cv::destroyAllWindows();
    if (videoWriter_.isOpened()) {
        videoWriter_.release();
    }
}
