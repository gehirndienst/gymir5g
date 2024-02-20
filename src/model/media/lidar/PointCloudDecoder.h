/*
 * PointCloudDecoder.h
 *
 * decoder for point clouds serialized with boost
 *
 * Created on: Feb 22, 2023
 *      Author: Nikita Smirnov
 */
#ifndef POINTCLOUDDECODER_H
#define POINTCLOUDDECODER_H

#include <iostream>

#include <boost/archive/binary_iarchive.hpp>

#include "model/media/lidar/PointCloudEncoder.h"
#include "model/media/lidar/VLPLidarGrabber.h"
#include "model/presets/PointCloudCompressionPresets.h"

class PointCloudDecoder {
    // for packets transferring
    static const int MAX_PACKET_SIZE = 64000;
public:
    PointCloudDecoder(int verbose = 0);
    PointCloudDecoder(const PointCloudDecoder& other);
    virtual ~PointCloudDecoder();

    void init(std::string viewerName = "pclDecoder");
    bool decode(uint8_t* data, int size);
    void clearAssemblingContainer();

    void view(CloudPtr& cloud);
    void view();

    CloudPtr& getDecodedCloud();
    int getCloudsDecoded();

    void flush();

public:
    bool isViewerStopped;
    bool isViewerStarted;
    bool isOk;
    int verbose;
private:
    CloudPtr assembledCloud_;
    CloudPtr decodedCloud_;
    std::stringstream serializedData_;
    pcl::visualization::PCLVisualizer::Ptr viewer_;
    pcl::visualization::PointCloudColorHandlerGenericField<Point>::Ptr colorHandler_;
    std::string viewerName_;
    int cloudsDecoded_;
};

#endif