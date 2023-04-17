#include "config.hpp"

#include "imageprocessor.hpp"

#ifndef IMAGE_PROCESSOR_SORT_EPSILON
#define IMAGE_PROCESSOR_SORT_EPSILON ((ImageProcessor::width/100) >= 4 ? (ImageProcessor::width/100) : 4)
#endif

#include <cmath>
#include <cstdio>

ImageProcessor::ImageProcessor() {
    width = 0;
    height = 0;
    data = nullptr;
    points = nullptr;
    numPoints = 0;
}

ImageProcessor::ImageProcessor(const unsigned char *grayscale, unsigned width, unsigned height) {
    data = nullptr;
    points = nullptr;
    numPoints = 0;
    setImage(grayscale, width, height);
}

ImageProcessor::~ImageProcessor() {
    if(data != nullptr) {
        delete[] data;
        data = nullptr;
    }

    if(points != nullptr) {
        delete[] points;
        points = nullptr;
    }
}

//1 byte per pixel, row-major order. Does deep copy of data
void ImageProcessor::setImage(const unsigned char *grayscale, unsigned width, unsigned height) {
    if(data != nullptr) {
        delete[] data;
        data = nullptr;
    }

    this->width = width;
    this->height = height;
    data = new unsigned char[width*height];
    numPoints = 0;

    for(unsigned i = 0; i < width*height; i++) {
        data[i] = grayscale[i];
    }
}

unsigned char *ImageProcessor::getResult(unsigned &width, unsigned &height) {
    width = this->width;
    height = this->height;
    return data;
}

void ImageProcessor::preprocess() {
    if(data == nullptr)
        return;

    /*
    float average = 0;
    int min = 255;
    int max = 0;

    for(int y = 0; y < (int) height; y++) {
        for(int x = 0; x < (int) width; x++) {
            if(data[width*y + x] > max) {
                max = data[width*y + x];
            }

            if(data[width*y + x] < min) {
                min = data[width*y + x];
            }

            average += data[width*y + x];
        }
    }

    if(width * height != 0)
        average /= width * height;

    int mid = (max + min) / 2;

    int zeros = 0;
    int ones = 0;

    for(int y = 0; y < (int) height; y++) {
        for(int x = 0; x < (int) width; x++) {
            data[width*y + x] = (data[width*y + x] > (average + mid) / 2) * 255; //convert to monochrome
            if(data[width*y + x] == 0) {
                zeros++;
            }
            else {
                ones++;
            }
        }
    }

    printf("zeroes: %d, ones: %d, max: %d, min: %d, avg: %f\n", zeros, ones, max, min, average);
    fflush(stdout);
    */

    int kernelSize = 5;
    float kernel[kernelSize*kernelSize];

    for(int y = 1; y < kernelSize - 1; y++) {
        for(int x = 1; x < kernelSize - 1; x++) {
            kernel[kernelSize*y + x] = 0.5;
        }
    }

    for(int i = 0; i < kernelSize; i++) {
        kernel[i] = -0.5;
        kernel[(kernelSize-1)*kernelSize+i] = -0.5;
        kernel[kernelSize*i] = -0.5;
        kernel[kernelSize*i+kernelSize-1] = -0.5;
    }


    for(int i = 0; i < 1; i++) {
        crossCorrelate(kernel, kernelSize, kernelSize, true);

        for(int y = 0; y < (int) height; y++) {
            for(int x = 0; x < (int) width; x++) {
                data[width*y + x] = (data[width*y + x] > 160) * 255; //convert to monochrome
            }
        }
    }
}

//kernel is in row major order
void ImageProcessor::crossCorrelate(const float *kernel, unsigned kw, unsigned kh, bool pad) {

    //output width and height
    unsigned ccw = width;
    unsigned cch = height;

    //kernel radius (half kernel width or height)
    int hkw = kw/2;
    int hkh = kh/2;

    //if input image is not padded, result is smaller than input
    if(!pad) {
        ccw -= hkw*2;
        cch -= hkh*2;
    }

    //cross correlation output (initialized to 0)
    float *cc = new float[ccw * cch] {}; //can optimize allocation here

    //cross correlation variables
    float kVal;
    int ix;
    int iy;

    //kernel is outermost loop for optimal performance
    for(int ky = -hkh; ky < hkh; ky++) {
        for(int kx = -hkw; kx < hkw; kx++) {
            kVal = kernel[kw*(ky+hkh) + (kx+hkw)];

            //loop through all pixels in output
            for(unsigned y = 0; y < cch; y++) {
                iy = y+ky + (pad ? hkh : 0);
                if(iy < 0 || iy > (int) height) continue;

                for(unsigned x = 0; x < ccw; x++) {
                    ix = x+kx + (pad ? hkw : 0);
                    if(ix < 0 || ix > (int) width) continue;

                    cc[ccw*y + x] += kVal * data[width*iy + ix];
                }
            }
        }
    }

    float maxVal = cc[0];
    float minVal = cc[0];

    //loop through all pixels in output and find min and max values
    for(unsigned y = 0; y < cch; y++) {
        for(unsigned x = 0; x < ccw; x++) {
            if(cc[ccw*y + x] > maxVal) {
                maxVal = cc[ccw*y + x];
            }

            if(cc[ccw*y + x] < minVal) {
                minVal = cc[ccw*y + x];
            }
        }
    }

    width = ccw;
    height = cch;
    delete[] data;
    data = new unsigned char[width * height];

    //normalize to 8-bit grayscale and write to output
    for(unsigned y = 0; y < height; y++) {
        for(unsigned x = 0; x < width; x++) {
            data[width*y + x] = ((cc[ccw*y + x] - minVal) / (maxVal - minVal) * 255);
        }
    }

    delete[] cc;
}

int ImageProcessor::threshold(unsigned n) {
    if(points != nullptr) {
        delete[] points;
    }

    points = new Point[n] {};

    //binary search with two thresholds
    int bottomLo = 0;
    int bottomHi = 255;
    int bottomMid;
    unsigned bottomCount;

    int topLo = 0;
    int topHi = 255;
    int topMid;
    unsigned topCount;

    while(topHi >= topLo || bottomHi >= bottomLo) {
        bottomCount = 0;
        topCount = 0;
        bottomMid = (bottomHi+bottomLo)/2;
        topMid = (topHi+topLo)/2;

#ifdef DEBUG_MODE_IMAGE_PROCESSOR
        printf("[ImageProcessor] threshold(n=%d) topMid=%d, bottomMid=%d\n", n, topMid, bottomMid);
        fflush(stdout);
#endif

        //loop through all pixels
        for(unsigned y = 0; y < height; y++) {
            for(unsigned x = 0; x < width; x++) {
                if(data[width*y + x] > bottomMid) {
                    if(bottomCount < n) {
                        points[bottomCount] = {(int)x, (int)y};
                    }
                    else if(topCount > n) {
                        goto narrow; //no regrets
                    }

                    bottomCount++;
                }

                if(data[width*y + x] > topMid) {
                    if(topCount < n) {
                        points[topCount] = {(int)x, (int)y};
                    }
                    else if(bottomCount > n) {
                        goto narrow; //still no regrets
                    }

                    topCount++;
                }
            }
        }

        narrow:
        if(bottomHi >= bottomLo) {
            if(bottomCount > n) { //threshold too low
                bottomLo = bottomMid + 1;
            }
            else { //threshold too high or just right; decrease for lower bound
                bottomHi = bottomMid - 1;

                if(bottomCount == n)
                    numPoints = bottomCount;
            }
        }

        if(topHi >= topLo) {
            if(topCount < n) { //threshold too high
                topHi = topMid - 1;
            }
            else { //threshold too low or just right; increase for upper bound
                topLo = topMid + 1;

                if(topCount == n)
                    numPoints = topCount;
            }
        }
    }

    if(topMid < bottomMid) {
        numPoints = 0;
    }

    return topMid - bottomMid;
}

//sorts points by X coordinate if Y values are similar
ImageProcessor::Point *ImageProcessor::sortPoints(unsigned &nPoints) {
    if(points == nullptr) {
        nPoints = 0;
        return nullptr;
    }

    Point tmp;

    int span = 1;
    for(int i = 0; i+span < (int) numPoints; i++) {
        while(i+span < (int) numPoints && points[i+span].y - points[i].y < (int) IMAGE_PROCESSOR_SORT_EPSILON) {
            span++;
        }

        printf("I am sorting %d points and my indices are i=%d and span=%d\n", numPoints, i, span);
        fflush(stdout);

        //if we have multiple points that have similar y-coordinates, sort them by x
        if(span > 1) {
            //Insertion sort is an O(n^2) algorithm but it's okay with just a few elements
            for(int j = 0; j < span; j++) {
                tmp = points[i+j];
                int k;

                for(k = j-1; k >= 0 && points[i+k].x > tmp.x; k--) {
                    points[i+k+1] = points[i+k];
                }

                points[i+k+1] = tmp;
            }

            //the range from i to i+span-1 is sorted, so we can increase i by span
            i += span;
        }

        span = 1;
    }

    nPoints = numPoints;
    return points;
}

struct ImageProcessor::Point *ImageProcessor::scalePoints(Point *to) {
    if(numPoints == 0 || points == nullptr) {
        return {};
    }

    float originalPerimeter = 0;
    float targetPerimeter = 0;

    float dx, dy;

    for(unsigned i = 0; i < numPoints - 1; i++) {
        dx = points[i].x - points[i+1].x;
        dy = points[i].y - points[i+1].y;
        originalPerimeter += sqrt(dx*dx + dy*dy);

        dx = to[i].x - to[i+1].x;
        dy = to[i].y - to[i+1].y;
        targetPerimeter += sqrt(dx*dx + dy*dy);
    }

    dx = points[numPoints-1].x - points[0].x;
    dy = points[numPoints-1].y - points[0].y;
    originalPerimeter += sqrt(dx*dx + dy*dy);

    dx = to[numPoints-1].x - to[0].x;
    dy = to[numPoints-1].y - to[0].y;
    targetPerimeter += sqrt(dx*dx + dy*dy);

    for(unsigned i = 0; i < numPoints - 1; i++) {
        points[i].x *= targetPerimeter / originalPerimeter;
        points[i].y *= targetPerimeter / originalPerimeter;
    }

    return points;
}

struct ImageProcessor::Point ImageProcessor::calcDisplacement(Point *from) {
    if(numPoints == 0 || points == nullptr) {
        return {};
    }

    int x = 0;
    int y = 0;

    for(unsigned i = 0; i < numPoints; i++) {
        x += points[i].x - from[i].x;
        y += points[i].y - from[i].y;
    }

    return Point {x / (int) numPoints, y / (int) numPoints};
}
