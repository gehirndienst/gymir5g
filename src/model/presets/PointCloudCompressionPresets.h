/*
 * PoinCloudCompressionPresets.h
 *
 * presets for point cloud compression
 * based on the profiles from PCL library and depends on it
 * see https://pcl.readthedocs.io/projects/tutorials/en/latest/compression.html#octree-compression for more info
 *
 * Created on: June 15, 2022
 *      Author: Nikita Smirnov
 */

#include <stdexcept>
#include <pcl/compression/octree_pointcloud_compression.h>

#pragma once

enum PointCloudQuality {
    VERY_LOW,
    LOW,
    MEDIUM,
    HIGH,
    VERY_HIGH,
    DEFAULT
};

inline int POINT_CLOUD_QUALITY_MAX = 5; 

inline PointCloudQuality getPointCloudQualityByIndex(int index) {
    if (index < 0 || index >= 6)
        throw std::invalid_argument("PoinCloudCompressionPresets::getPointCloudQualityByIndex wrong index, should be 0--5");
    switch(index) {
        case 0:
            return VERY_LOW;
        case 1:
            return LOW;
        case 2:
            return MEDIUM;
        case 3:
            return HIGH;
        case 4:
            return VERY_HIGH;
        default:
            return DEFAULT;
    }
}

inline std::string getPointCloudQualityStringByIndex(int index) {
    if (index < 0 || index >= 6)
        throw std::invalid_argument("PoinCloudCompressionPresets::getPointCloudQualityStringByIndex wrong index, should be 0--5");
    switch(index) {
        case 0:
            return "VERY_LOW";
        case 1:
            return "LOW";
        case 2:
            return "MEDIUM";
        case 3:
            return "HIGH";
        case 4:
            return "VERY_HIGH";
        default:
            return "DEFAULT";
    }  
}

inline PointCloudQuality getPointCloudQualityByBitrate(double bitrate) {
    // same as for VideoQualityPresets to be sustainable
    if (bitrate < 850) return VERY_LOW;
    else if (bitrate >= 850 && bitrate < 2250) return LOW;
    else if (bitrate >= 2250 && bitrate < 4500) return MEDIUM;
    else if (bitrate >= 4500 && bitrate < 10000) return HIGH;
    else return VERY_HIGH;
}

// choose quality preset and whether fast/slow encoding 
// Quality: better quality - better point resolution, but bigger file
// Fast/Slow: fast but the size is bigger  slow needs time but compresses much more effectively
inline pcl::io::configurationProfile_t getCompressionProfile(PointCloudQuality preset, bool isFastCompressing) {
    switch (preset) {
        case VERY_LOW:
            if (isFastCompressing) {
                return {
                    // MANUAL CONFIGURATION FOR VERY LOW RESOLUTION ONLINE
                    0.05, /* pointResolution = */
                    0.05, /* octreeResolution = */
                    true, /* doVoxelGridDownDownSampling = */
                    70, /* iFrameRate = */
                    4, /* colorBitResolution = */
                    true /* doColorEncoding = */
                    };
            } else {
                return {
                    // MANUAL CONFIGURATION FOR VERY LOW RESOLUTION OFFLINE
                    0.05, /* pointResolution = */
                    0.05, /* octreeResolution = */
                    true, /* doVoxelGridDownDownSampling = */
                    100, /* iFrameRate = */
                    4, /* colorBitResolution = */
                    true /* doColorEncoding = */
                    };
            }
        case LOW:
            if (isFastCompressing) {
                return {
                    // PROFILE: LOW_RES_ONLINE_COMPRESSION_WITH_COLOR
                    0.01, /* pointResolution = */
                    0.01, /* octreeResolution = */
                    true, /* doVoxelGridDownDownSampling = */
                    50, /* iFrameRate = */
                    4, /* colorBitResolution = */
                    true /* doColorEncoding = */
                    };
            } else {
                return {
                    // PROFILE: LOW_RES_OFFLINE_COMPRESSION_WITH_COLOR
                    0.01, /* pointResolution = */
                    0.01, /* octreeResolution = */
                    true, /* doVoxelGridDownDownSampling = */
                    100, /* iFrameRate = */
                    4, /* colorBitResolution = */
                    true /* doColorEncoding = */
                    };
            }
        case MEDIUM:
            if (isFastCompressing) {
                return {
                    // PROFILE: MED_RES_ONLINE_COMPRESSION_WITH_COLOR
                    0.005, /* pointResolution = */
                    0.01, /* octreeResolution = */
                    false, /* doVoxelGridDownDownSampling = */
                    40, /* iFrameRate = */
                    5, /* colorBitResolution = */
                    true /* doColorEncoding = */
                    }; 
            } else {
                return {
                    // PROFILE: MED_RES_OFFLINE_COMPRESSION_WITH_COLOR
                    0.005, /* pointResolution = */
                    0.005, /* octreeResolution = */
                    true, /* doVoxelGridDownDownSampling = */
                    100, /* iFrameRate = */
                    5, /* colorBitResolution = */
                    true /* doColorEncoding = */
                };
            }
        case HIGH:
            if (isFastCompressing) {
                return {
                    // PROFILE: HIGH_RES_ONLINE_COMPRESSION_WITH_COLOR
                    0.0001, /* pointResolution = */
                    0.01, /* octreeResolution = */
                    false, /* doVoxelGridDownDownSampling = */
                    30, /* iFrameRate = */
                    7, /* colorBitResolution = */
                    true /* doColorEncoding = */
                };
            } else {
                return {
                    // PROFILE: HIGH_RES_OFFLINE_COMPRESSION_WITH_COLOR
                    0.0001, /* pointResolution = */
                    0.0001, /* octreeResolution = */
                    true, /* doVoxelGridDownDownSampling = */
                    100, /* iFrameRate = */
                    8, /* colorBitResolution = */
                    true /* doColorEncoding = */
                };
            }
        case VERY_HIGH:
            if (isFastCompressing) {
                return {
                    // MANUAL CONFIGURATION FOR VERY HIGH RESOLUTION ONLINE
                    0.00005, /* pointResolution = */
                    0.001, /* octreeResolution = */
                    false, /* doVoxelGridDownDownSampling = */
                    20, /* iFrameRate = */
                    8, /* colorBitResolution = */
                    true /* doColorEncoding = */
                };
            } else {
                return {
                    // MANUAL CONFIGURATION FOR VERY HIGH RESOLUTION OFFLINE
                    0.00005, /* pointResolution = */
                    0.00005, /* octreeResolution = */
                    true, /* doVoxelGridDownDownSampling = */
                    100, /* iFrameRate = */
                    8, /* colorBitResolution = */
                    true /* doColorEncoding = */
                };
            }
        default:
        // for DEFAULT use MEDIUM with fast encoding (basically this is a decoder)
            return {
                // PROFILE: MED_RES_ONLINE_COMPRESSION_WITH_COLOR
                0.005, /* pointResolution = */
                0.01, /* octreeResolution = */
                false, /* doVoxelGridDownDownSampling = */
                40, /* iFrameRate = */
                5, /* colorBitResolution = */
                true /* doColorEncoding = */
            };
    }
}

