#include "PointCloudDecoder.h"

PointCloudDecoder::PointCloudDecoder(int verbose) 
    :isViewerStopped(false)
    ,isViewerStarted(false)
    ,isOk(false)
    ,verbose(verbose)
    ,assembledCloud_(CloudPtr(new Cloud))
    ,cloudsDecoded_(0) {}

PointCloudDecoder::PointCloudDecoder(const PointCloudDecoder& other) {}

PointCloudDecoder::~PointCloudDecoder() { flush(); }

void PointCloudDecoder::init(std::string viewerName) {
    viewerName_ = viewerName;
    isOk = true;
}

bool PointCloudDecoder::decode(uint8_t* buffer, int size) {
    serializedData_.str(std::string((char*)buffer, size));
    boost::archive::binary_iarchive ia(serializedData_);
    PointCloudSerializer serializer;
    ia >> serializer;
    decodedCloud_ = serializer.getCloud();
    *assembledCloud_ += *decodedCloud_;
    return decodedCloud_->size() > 0;
}

void PointCloudDecoder::clearAssemblingContainer() {
    assembledCloud_->clear();
}

void PointCloudDecoder::view(CloudPtr& cloud) {
    if (viewer_) {
        // display the cloud
        isViewerStopped = viewer_->wasStopped();
        if (!isViewerStopped) {
            viewer_->spinOnce();
            if (!cloud->empty()) {
                colorHandler_->setInputCloud(cloud);
                if (!viewer_->updatePointCloud(cloud, *colorHandler_)) {
                    viewer_->addPointCloud<Point>(cloud, *colorHandler_);
                }
            } else {
                viewer_->spinOnce();
            }
        }
        if (!isViewerStarted) isViewerStarted = true;
    } else {
            // create viewer
        viewer_ = pcl::visualization::PCLVisualizer::Ptr(
            new pcl::visualization::PCLVisualizer(viewerName_)
        );
        viewer_->addCoordinateSystem(1.0, "coordinate");
        viewer_->setBackgroundColor(0.0, 0.0, 0.0, 0);
        viewer_->initCameraParameters();
        viewer_->setCameraPosition(0.0, 0.0, 30.0, 0.0, 1.0, 0.0, 0);
        viewer_->setCameraClipDistances(0.0, 50.0);

        colorHandler_ = pcl::visualization::PointCloudColorHandlerGenericField<Point>::Ptr(
            new pcl::visualization::PointCloudColorHandlerGenericField<Point>("intensity")
        );
    }
}

void PointCloudDecoder::view() { view(assembledCloud_); }

int PointCloudDecoder::getCloudsDecoded() { return cloudsDecoded_; }

CloudPtr& PointCloudDecoder:: getDecodedCloud() { return decodedCloud_; }

void PointCloudDecoder::flush() {
    assembledCloud_.reset();
    decodedCloud_.reset();
    if (isViewerStarted) {
        viewer_->close();
    }
    viewer_.reset();
    colorHandler_.reset();
    cloudsDecoded_ = 0;
}
