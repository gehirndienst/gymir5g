/*
 * QualityOfContent.h
 *
 * class for useful quality metrics
 *
 *  Created on: Jul 10, 2023
 *      Author: Nikita Smirnov
 */

#ifndef QUALITYOFCONTENT_H
#define QUALITYOFCONTENT_H

class QualityOfContent {
public:
    virtual ~QualityOfContent() = default;
};

struct QualityOfVideo : public QualityOfContent {
    double crf;
    double psnr;
    double ssim;
};

#endif