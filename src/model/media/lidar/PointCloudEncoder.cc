#include "PointCloudEncoder.h"

PointCloudEncoder::PointCloudEncoder(int maxPacketSize, int verbose)
    :maxCloudSize(maxPacketSize)
    ,maxPointsInOnePacket(std::floor(static_cast<float>(maxCloudSize)/sizeof(Point)))
    ,isOk(false)
    ,verbose(verbose)
    ,rgbCloud_(pcl::PointCloud<pcl::PointXYZRGB>::Ptr(new pcl::PointCloud<pcl::PointXYZRGB>))
    ,initialCloud_(CloudPtr(new Cloud))
    ,cloudsProcessed_(0)
    ,compressedCloudSize_(0)
    ,encodedDataSize_(0)
{}

PointCloudEncoder::PointCloudEncoder(const PointCloudEncoder& other) {}

PointCloudEncoder::~PointCloudEncoder() {
    if (octreeCompressor_) {
        flush();
    }
}

void PointCloudEncoder::init(PointCloudQuality preset, bool isFastCompressing) {
    flush();

    if (preset == DEFAULT) {
        // decoder
        octreeCompressor_ = std::unique_ptr<pcl::io::OctreePointCloudCompression<pcl::PointXYZRGB>>(
            new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGB>()
        );
    } else {
        // encoder
        auto profile = getCompressionProfile(preset, isFastCompressing);
        octreeCompressor_ = std::unique_ptr<pcl::io::OctreePointCloudCompression<pcl::PointXYZRGB>>(
            new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGB>(
                pcl::io::MANUAL_CONFIGURATION,
                false,
                profile.pointResolution,
                profile.octreeResolution,
                profile.doVoxelGridDownSampling,
                profile.iFrameRate,
                profile.doColorEncoding,
                profile.colorBitResolution
            )
        );
    }

    if (!octreeCompressor_) {
        throw std::runtime_error("[PointCloudEncoder::PointCloudEncoder] Octree encoder can not start");
    } else {
        isOk = true;
        if (verbose > 1) std::cout << "[PointCloudEncoder::PointCloudEncoder] Octree encoder is succesfully started, parameters: \nQUALITY: " 
            << preset << "\nEncoding: " << (isFastCompressing ? "fast" : "slow") << std::endl;
    }
}

void PointCloudEncoder::init() {
    init(PointCloudQuality::DEFAULT, true);
}

void PointCloudEncoder::encode(Cloud& input) {
    encodedDataSize_ = 0;
    // compress input cloud with a given preset
    if (!compress(input)) {
        std::cerr << "PointCloudEncoder::encode encoding fails on 1. compression phase!" << std::endl;
        return;
    }
    // decompress back to cloud object to preserve intergrity
    if (!decompress()) {
        std::cerr << "PointCloudEncoder::encode encoding fails on 2. decompression phase!" << std::endl;
        return;
    }
    cloudsProcessed_ ++;
    // split according to max packet size
    splitToMultipleClouds();
    // serialize and fill returning vector
    for (auto& cloudPtr : splittedCloudsVector_) {
        serialize(cloudPtr);
    }
}

bool PointCloudEncoder::compress(Cloud& input) {
    // compress
    fromInputFormatToRGB(input, *rgbCloud_);
    clearStringStream(compressedData_);
    octreeCompressor_->encodePointCloud(rgbCloud_, compressedData_);

    // retrieve compressed size
    compressedData_.seekg(0, ios::end);
    compressedCloudSize_ = compressedData_.tellg();
    compressedData_.seekg(0, ios::beg);
    if (compressedCloudSize_ <= 0) {
        std::cerr << "[PointCloudEncoder::compress] compressing fails on pcl N.: " << cloudsProcessed_ << std::endl;
        return false;
    }
    return true;
}

bool PointCloudEncoder::decompress() {
    octreeCompressor_->decodePointCloud(compressedData_, rgbCloud_);
    if (rgbCloud_->empty()) {
        std::cerr << "[PointCloudEncoder::decompress] decompressing fails on pcl N.: " << cloudsProcessed_ << std::endl;
        return false;
    }

    // finalize and save the output in initialCloud
    fromRGBToInputFormat(*rgbCloud_, *initialCloud_);
    return true;
}

void PointCloudEncoder::splitToMultipleClouds() {
    clearContainers();
    int cloudSize = (int)initialCloud_->size();
    int packetsNum = std::ceil(static_cast<float>(cloudSize)/maxPointsInOnePacket);
    int offset = 0;
    for (int i = 0; i < packetsNum; i++) {
        int pointsInPacket = std::min(std::abs(cloudSize - offset), maxPointsInOnePacket);
        CloudPtr newCloud (new Cloud);
        newCloud->width = pointsInPacket;
        newCloud->height = 1;
        newCloud->is_dense = false;
        newCloud->points.resize(newCloud->width * newCloud->height);
        for (size_t j = 0; j < (size_t)pointsInPacket; j++) {
            size_t cloudIndex = offset + j;
            newCloud->points[j].x = initialCloud_->at(cloudIndex).x;
            newCloud->points[j].y = initialCloud_->at(cloudIndex).y;
            newCloud->points[j].z = initialCloud_->at(cloudIndex).z;
            newCloud->points[j].intensity = initialCloud_->at(cloudIndex).intensity;
        }
        offset += pointsInPacket;
        splittedCloudsVector_.push_back(newCloud);
    }
}

void PointCloudEncoder::serialize(CloudPtr& cloudPtr) {
    // serialize a cloud (boost serialization is stuck to std streams)
    boost::archive::binary_oarchive oa(serializedData_);
    PointCloudSerializer serializer;
    serializer.setCloud(cloudPtr);
    oa << serializer;
    serializedData_.seekg(0, ios::end);
    int serializedSize = serializedData_.tellg();
    serializedData_.seekg(0, ios::beg);
    encodedDataSize_ += serializedSize;
    
    // fill into encoded data vector
    char* buffer = new char[serializedSize];
    serializedData_.read(buffer, serializedSize);
    clearStringStream(serializedData_);
    encodedClouds_.push_back(std::make_pair(reinterpret_cast<uint8_t*>(buffer), serializedSize));
}

std::vector<std::pair<uint8_t*, int>>& PointCloudEncoder::getEncodedPointClouds() {
    return encodedClouds_;
}

void PointCloudEncoder::getCompressedPointCloud(uint8_t* buffer) {
    char* charBuffer = new char[compressedCloudSize_];
    compressedData_.read(charBuffer, compressedCloudSize_);
    clearStringStream(compressedData_);
    memcpy(buffer, reinterpret_cast<uint8_t *>(charBuffer), compressedCloudSize_);
}


CloudPtr& PointCloudEncoder::getDecompressedPointCloud() {
    return initialCloud_;
}

int PointCloudEncoder::getEncodedDataSize() {
    return encodedDataSize_;
}

int PointCloudEncoder::getCompressedCloudSize() {
    return compressedCloudSize_;
}

int PointCloudEncoder::getCloudsProcessed() {
    return cloudsProcessed_;
}

void PointCloudEncoder::clearStringStream(std::stringstream& stream) {
    stream.str(std::string());
    stream.clear();
}

void PointCloudEncoder::clearContainers() {
    for (auto& cloudPtr : splittedCloudsVector_) {
        cloudPtr.reset();
    }
    splittedCloudsVector_.clear();
    for (auto& bufferPair : encodedClouds_) {
        if (bufferPair.first) {
            delete bufferPair.first;
        }
    }
    encodedClouds_.clear();
}

void PointCloudEncoder::fromInputFormatToRGB(Cloud& input, pcl::PointCloud<pcl::PointXYZRGB>& output) {
    // the idea is to encode intensity as "R" value
    if (!output.empty()) {
        output.clear();
    }
    output.width = input.width;
    output.height = input.height;
    for (const auto& iPoint : input) {
        pcl::PointXYZRGB rgbPoint;
        rgbPoint.x = iPoint.x;
        rgbPoint.y = iPoint.y; 
        rgbPoint.z = iPoint.z;
        rgbPoint.r = static_cast<uint8_t>(iPoint.intensity);
        output.push_back(rgbPoint);
    }  
}

void PointCloudEncoder::fromRGBToInputFormat(pcl::PointCloud<pcl::PointXYZRGB>& input, Cloud& output) {
    // for viewer
    if (!output.empty()) {
        output.clear();
    }
    output.width = input.width;
    output.height = input.height;
    for (const auto& rgbPoint : input) {
        pcl::PointXYZI iPoint;
        iPoint.x = rgbPoint.x;
        iPoint.y = rgbPoint.y; 
        iPoint.z = rgbPoint.z;
        iPoint.intensity = static_cast <float>(rgbPoint.r);
        output.push_back(iPoint);
    }
}

void PointCloudEncoder::flush() {
    clearContainers();
    clearStringStream(compressedData_);
    clearStringStream(serializedData_);

    if (!rgbCloud_->empty()) {
        rgbCloud_->clear();
    }
    if (!initialCloud_->empty()) {
        initialCloud_->clear();
    }
    octreeCompressor_ = nullptr;
    compressedCloudSize_ = 0;
    encodedDataSize_ = 0;
}

// SERIALIZER

void PointCloudSerializer::setCloud(CloudPtr& cloud) {
    // NOTE: a flattened pcl structure is assumed here, i.e., height = 1
    for (const auto& point : (*cloud)) {
        x_.push_back(point.x);
        y_.push_back(point.y);
        z_.push_back(point.z);
        i_.push_back(point.intensity);
        width_ = (int)cloud->width;
    }  
}

CloudPtr& PointCloudSerializer::getCloud() {
    cloud->clear();
    cloud->width = width_;
    cloud->height = 1;
    cloud->is_dense = false;
    cloud->points.resize(cloud->width * cloud->height);
    for (size_t i = 0; i < (size_t)width_; i++) {
        cloud->points[i].x = x_[i];
        cloud->points[i].y = y_[i];
        cloud->points[i].z = z_[i];
        cloud->points[i].intensity = i_[i];
    }
    return cloud;
}