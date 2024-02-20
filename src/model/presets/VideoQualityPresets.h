/*
 * VideoQualityPresets.h
 *
 * presets for video quality for adaptive streaming like, e.g., in Youtube
 * parameters for presets are tuned after several tests and secondary literature review
 *
 * Created on: July 7, 2022
 *      Author: Nikita Smirnov
 */

#include <iostream>
#include <stdexcept>
#include <string>

#pragma once

enum VideoQuality {
    LD_240,
    SD_480,
    HD_720,
    FHD_1080,
    UHD_2160
};

enum VideoEncodingSpeed {
    VERY_SLOW,
    SLOW,
    MODERATE,
    FAST,
    VERY_FAST
};

inline int VIDEO_QUALITY_MAX = 5; 

struct VideoQualityProfile {
    int dstWidth;
    int dstHeight; 
    int bitrateKbps;
    std::string h264Profile;
    std::string h264Preset;
};

inline VideoQuality getVideoQualityByIndex(int index) {
    if (index < 0 || index >= 5)
        throw std::invalid_argument("VideoQualityPresets::getVideoQualityByIndex wrong index, should be 0--4");
    switch(index) {
        case 0:
            return LD_240;
        case 1:
            return SD_480;
        case 2:
            return HD_720;
        case 3:
            return FHD_1080;
        case 4:
            return UHD_2160;
        default:
            return FHD_1080;
    }
}

inline std::string getVideoQualityStringByIndex(int index) {
    if (index < 0 || index >= 5)
        throw std::invalid_argument("VideoQualityPresets::getVideoQualityStringByIndex wrong index, should be 0--4");
    switch(index) {
        case 0:
            return "LD_240";
        case 1:
            return "SD_480";
        case 2:
            return "HD_720";
        case 3:
            return "FHD_1080";
        case 4:
            return "UHD_2160";
        default:
            return "ERROR";
    }
}

inline VideoQuality getVideoQualityByBitrate(double bitrate) {
    if (bitrate < 850) return LD_240;
    else if (bitrate >= 850 && bitrate < 2250) return SD_480;
    else if (bitrate >= 2250 && bitrate < 4500) return HD_720;
    else if (bitrate >= 4500 && bitrate < 10000) return FHD_1080;
    else return UHD_2160;
}

inline VideoEncodingSpeed getVideoEncodingSpeedByIndex(int index) {
    if (index < 0 || index >= 5)
        throw std::invalid_argument("VideoQualityPresets::getVideoEncodingSpeedByIndex wrong index, should be 0--4");
    switch(index) {
        case 0:
            return VERY_SLOW;
        case 1:
            return SLOW;
        case 2:
            return MODERATE;
        case 3:
            return FAST;
        case 4:
            return VERY_FAST;
        default:
            return FAST;
    }
}

inline VideoQualityProfile getH264VideoQualityProfile(VideoQuality quality, VideoEncodingSpeed speed) {
    VideoQualityProfile profile;

    switch (speed) {
        case VideoEncodingSpeed::VERY_SLOW:
            profile.h264Preset = "veryslow";
            break;
        case VideoEncodingSpeed::SLOW:
            profile.h264Preset = "slow";
            break;
        case VideoEncodingSpeed::MODERATE:
            profile.h264Preset = "medium";
            break;
        case VideoEncodingSpeed::FAST:
            profile.h264Preset = "veryfast";
            break;
        case VideoEncodingSpeed::VERY_FAST:
            profile.h264Preset = "ultrafast";
            break;
        default:
            profile.h264Preset = "medium";
            break;
    }

    switch (quality) {
        case VideoQuality::LD_240:
            profile.dstWidth = 480;
            profile.dstHeight = 272;
            profile.bitrateKbps = 400;
            profile.h264Profile = "baseline";
            break;
        case VideoQuality::SD_480:
            profile.dstWidth = 854;
            profile.dstHeight = 480;
            profile.bitrateKbps = 1500;
            profile.h264Profile = "main";
            break;
        case VideoQuality::HD_720:
            profile.dstWidth = 1280;
            profile.dstHeight = 720;
            profile.bitrateKbps = 3000;
            profile.h264Profile = "main";
            break;
        case VideoQuality::FHD_1080:
            profile.dstWidth = 1920;
            profile.dstHeight = 1080;
            profile.bitrateKbps = 6000;
            profile.h264Profile = "main";
            break;
        case VideoQuality::UHD_2160:
            profile.dstWidth = 3840;
            profile.dstHeight = 2160;
            profile.bitrateKbps = 15000;
            profile.h264Profile = "high";
            break;  
        default:
            // 4k
            profile.dstWidth = 3840;
            profile.dstHeight = 2160;
            profile.bitrateKbps = 15000;
            profile.h264Profile = "high";
            break;  
    }

    return profile;
}

