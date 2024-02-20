/*
 * PointCloudEncoder.h
 *
 * octree compressor/decompressor for pcl PointCloud (not PointCloud2)
 *
 * Created on: June 14, 2022
 *      Author: Nikita Smirnov
 */
#ifndef POINTCLOUDENCODER_H
#define POINTCLOUDENCODER_H

#include <iostream>

#include <pcl/common/io.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/vlp_grabber.h>
#include <pcl/compression/octree_pointcloud_compression.h>
#include <pcl/visualization/pcl_visualizer.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include "model/presets/PointCloudCompressionPresets.h"
#include "model/media/lidar/VLPLidarGrabber.h"

class PointCloudSerializer {
public:
    PointCloudSerializer(): cloud(CloudPtr(new Cloud)) {}
    virtual ~PointCloudSerializer() { cloud.reset(); }

    CloudPtr& getCloud();
    void setCloud(CloudPtr& cloud);
protected:
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& ar, const unsigned int /* version */ ) { ar& x_ & y_ & z_ & i_ & width_; }
protected:
    CloudPtr cloud;
private:
    std::vector<float> x_;
    std::vector<float> y_;
    std::vector<float> z_;
    std::vector<float> i_;
    int width_;
};

class PointCloudEncoder {
    // for packets transferring
    static const int MAX_PACKET_SIZE = 64000;
public:
    PointCloudEncoder(int maxPacketSize = 64000, int verbose = 0);
    PointCloudEncoder(const PointCloudEncoder& other);
    virtual ~PointCloudEncoder();

    void init(PointCloudQuality preset, bool isFastCompressing);
    void init();

    void encode(Cloud& input);

    std::vector<std::pair<uint8_t*, int>>& getEncodedPointClouds();
    void getCompressedPointCloud(uint8_t* buffer);
    CloudPtr& getDecompressedPointCloud();
    int getEncodedDataSize();
    int getCompressedCloudSize();
    int getCloudsProcessed();

    void flush();

private:
    bool compress(Cloud& input);
    bool decompress();
    void serialize(CloudPtr& cloudPtr);
    void splitToMultipleClouds();

    void fromInputFormatToRGB(Cloud& input, pcl::PointCloud<pcl::PointXYZRGB>& output);
    void fromRGBToInputFormat(pcl::PointCloud<pcl::PointXYZRGB>& input, Cloud& output);

    void clearStringStream(std::stringstream& stream);
    void clearContainers();

public:
    int maxCloudSize;
    int maxPointsInOnePacket;
    bool isOk;
    int verbose;

private:
    std::unique_ptr<pcl::io::OctreePointCloudCompression<pcl::PointXYZRGB>> octreeCompressor_;
    pcl::PointCloud<pcl::PointXYZRGB>::Ptr rgbCloud_;
    CloudPtr initialCloud_;
    std::vector<CloudPtr> splittedCloudsVector_;
    std::stringstream compressedData_;
    std::stringstream serializedData_;
    std::vector<std::pair<uint8_t*, int>> encodedClouds_;

    int cloudsProcessed_;
    int compressedCloudSize_;
    int encodedDataSize_;
};

#endif