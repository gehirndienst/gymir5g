#include "VideoEncoderH264.h"

VideoEncoderH264::VideoEncoderH264(int maxPacketSize, int verbose)
    : maxPacketSize(maxPacketSize)
    , isOk(false)
    , verbose(verbose)
    , enc_(nullptr)
    , picRaw_(nullptr)
    , sws_(nullptr)
    , isKeyFrame_(false)
    , framesEncoded_(0)
    , frameSize_(0)
{}

VideoEncoderH264::~VideoEncoderH264() {
    flush();
}

void VideoEncoderH264::init(VideoQuality quality, VideoEncodingSpeed speed, int srcWidth, int srcHeight) {
    flush();

    VideoQualityProfile profile = getH264VideoQualityProfile(quality, speed);
    srcWidth_ = srcWidth;
    srcHeight_ = srcHeight;

    // set dst width and height
    dstWidth_ = profile.dstWidth;
    dstHeight_ = profile.dstHeight;

    // SET x264 PARAMS:

    // set recommended profile and presets
    // FIXME: better to remove profile so that it will be figured out automatically?
    //x264_param_apply_profile(&prms_, profile.h264Profile.c_str());
    x264_param_default_preset(&prms_, profile.h264Preset.c_str(), "zerolatency,fastdecode");

    // set height, width, fps and timebase
    prms_.i_width = dstWidth_;
    prms_.i_height = dstHeight_;
    prms_.i_fps_num = FPS;
    prms_.i_timebase_num = 1;
    prms_.i_timebase_den = FPS;

    // image quality and NALs forming
    // min-max key int interval: the more it is, the bigger is the average bitrate, picture is smoother, but more dangerous to lost keyframe
    prms_.i_keyint_min = 1;
    prms_.i_keyint_max = FPS * 4;
    // nal size = packet size, send frames nal-by-nal to easily decode
    prms_.i_slice_max_size = std::min(maxPacketSize, X264_MAX_NAL_SIZE);

    prms_.i_threads = X264_THREADS_AUTO; // cpu threads for encoding

    // turn on estimating some useful video metrics
    prms_.analyse.i_me_method = X264_ME_HEX;
    prms_.analyse.b_ssim = 1;
    prms_.analyse.b_psnr = 1;

    /* some interesing x264 parameters, set by profile/preset by default  */
    // prms_.b_intra_refresh = 0; // flattens I and P frames sizes, e.g. instead 32k 4k 4k 4k 32k one has 12k 11k 11k 11k 12k
    // prms_.b_repeat_headers = 1; // must have, inludes PPS to every keyframe
    // prms_.b_annexb = 1; // places start code in front of NAL otherwise places the size
    // prms_.i_frame_reference = 1; // max amount of frames could be referenced, useful in case of packet loss
    // //prms_.i_bframe = 3; // how many B-frames max between two consecutive keyframes
    // prms_.i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

    // bitrate: adaptive bit rate mode (ABR) + constraint encoding (VBV) OR picture quality control (CRF params)
    // FIXME: use cbr instead with --nal-cbr?
    int bitrateKbps = profile.bitrateKbps;
    prms_.rc.i_rc_method = X264_RC_ABR; // turning if off influences a lot on the size of frames (x2-2,5 more)
    if (prms_.rc.i_rc_method == X264_RC_ABR) {
        // try to fit into bitrate (preferable)
        prms_.rc.i_bitrate = bitrateKbps;
        // turn on Video Buffering Verifier with Constrained encoding
        prms_.rc.i_vbv_max_bitrate = int(1.1 * bitrateKbps); // make == to bitrate to have CBR mode
        // expected client buffer size, stacks good with ultrafast+zerolatency
        prms_.rc.i_vbv_buffer_size = int(1.2 * bitrateKbps);
    } else if (prms_.rc.i_rc_method == X264_RC_CRF) {
        // use constant rate factor: try to stabilize the quality
        prms_.rc.f_rf_constant = 23;
        prms_.rc.f_rf_constant_max = 35;
    }

    // logging. NOTE: to obtain psnr/ssim metrics you must have X264_LOG_INFO level
    prms_.i_log_level = verbose >= 1 || prms_.analyse.b_psnr == 1 ? X264_LOG_INFO : X264_LOG_ERROR;

    prms_.i_csp = X264_CSP_I420;
    enc_ = x264_encoder_open(&prms_);
    x264_encoder_headers(enc_, &nals_, &nheader_);

    // alloc x264 pic
    x264_picture_alloc(&picIn_, X264_CSP_I420, dstWidth_, dstHeight_);

    // alloc frame for raw data
    picRaw_ = av_frame_alloc();
    if (!picRaw_) {
        std::cerr << "[VideoEncoderH264::VideoEncoderH264] Cannot create AVFrame" << std::endl;
        return;
    }

    // scaler + colors converter
    sws_ = sws_getContext(srcWidth_, srcHeight_, PIXEL_FORMAT, //AV_PIX_FMT_BGR24, AV_PIX_FMT_BAYER_GBRG8, others're worse
                          dstWidth_, dstHeight_, AV_PIX_FMT_YUV420P, // correct for h264
                          SWS_FAST_BILINEAR, NULL, NULL, NULL);

    if (!sws_) {
        std::cerr << "[VideoEncoderH264::VideoEncoderH264] Cannot create SWS context" << std::endl;
        return;
    }

    isOk = true;

    if (verbose > 1) std::cout << "H264 Encoder with following params: \n"
                                   << "source width:            " << srcWidth_ << "\n"
                                   << "source height:           " << srcHeight_ << "\n"
                                   << "destination width:       " << dstWidth_ << "\n"
                                   << "destination height:      " << dstHeight_ << "\n"
                                   << "fps:                     " << FPS << "\n"
                                   << "VBV mode bitrate (Kbps): " << bitrateKbps << "\n"
                                   << "Encoding speed:          " << profile.h264Preset << "\n"
                                   << "has been succesfully initiated" << std::endl;
}

bool VideoEncoderH264::changeBitrate(int bitrateKbps) {
    // works only with X264_RC_ABR and with CBR if rc.b_filler = 1 and rc.i_nal_hrd = 0
    x264_param_t newPrms;
    x264_encoder_parameters(enc_, &newPrms);
    newPrms.rc.i_rc_method = X264_RC_ABR;
    newPrms.rc.i_bitrate = bitrateKbps;
    newPrms.rc.i_vbv_max_bitrate = int(1.1 * newPrms.rc.i_bitrate);
    newPrms.rc.i_vbv_buffer_size = int(1.2 * newPrms.rc.i_bitrate);
    int error = x264_encoder_reconfig(enc_, &newPrms);
    if (error < 0) {
        std::cerr << "[VideoEncoderH264::changeBitrate] reconfigure has failed" << std::endl;
        return false;
    }
    return true;
}

bool VideoEncoderH264::encode(uint8_t* rawImgBuffer) {
    // save as AVPicture
    int picture = av_image_fill_arrays(picRaw_->data, picRaw_->linesize,
                                       rawImgBuffer, PIXEL_FORMAT, srcWidth_, srcHeight_, 1);
    if (!picture) {
        std::cerr << "[VideoEncoderH264::encode] raw data can't be filled into the buffer" << std::endl;
        return false;
    }

    // scale
    int sliceHeight = sws_scale(sws_, picRaw_->data, picRaw_->linesize, 0, srcHeight_,
                                picIn_.img.plane, picIn_.img.i_stride);
    if (sliceHeight != dstHeight_) {
        std::cerr << "[VideoEncoderH264::encode] rescale failed" << std::endl;
        return false;
    }

    // main encoding
    picIn_.i_pts = ++framesEncoded_;
    frameSize_ = x264_encoder_encode(enc_, &nals_, &nalsNum_, &picIn_, &picOut_);
    isKeyFrame_ = picOut_.i_type == 1;

    // fill container
    encodedNals_.clear();
    for (int i = 0; i < nalsNum_; i++) {
        encodedNals_.push_back(std::make_pair(nals_[i].p_payload, nals_[i].i_payload));
    }

    // fill qov
    if (prms_.analyse.b_psnr == 1 && prms_.i_log_level == X264_LOG_INFO) {
        qov_.crf = picOut_.prop.f_crf_avg;
        qov_.psnr = picOut_.prop.f_psnr_avg;
        qov_.ssim = picOut_.prop.f_ssim;
    }

    return true;
}

void VideoEncoderH264::getNals(uint8_t* buffer) {
    unsigned int tmpSize = 0;
    int i = 0;
    while (i < nalsNum_) {
        memcpy(&(buffer[tmpSize]), nals_[i].p_payload, nals_[i].i_payload);
        tmpSize += nals_[i].i_payload;
        i++;
    }
}

std::vector<std::pair<uint8_t*, int>>& VideoEncoderH264::getNals() { return encodedNals_; }

QualityOfVideo& VideoEncoderH264::getQov() { return qov_; }

int VideoEncoderH264::getFramesEncoded() { return framesEncoded_; }

int VideoEncoderH264::getFrameSize() { return frameSize_; }

bool VideoEncoderH264::isKeyFrame() { return isKeyFrame_; }

void VideoEncoderH264::flush() {
    encodedNals_.clear();
    if (picRaw_) {
        av_frame_free(&picRaw_);
    }
    if (sws_) {
        sws_freeContext(sws_);
    }
    if (enc_) {
        x264_encoder_close(enc_);
    }
}