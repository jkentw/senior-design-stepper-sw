#ifndef IMAGEPROCESSOR_HPP
#define IMAGEPROCESSOR_HPP

class ImageProcessor {

public:
    struct Point {
        int x;
        int y;
    };

    ImageProcessor();
    ImageProcessor(const unsigned char *grayscale, unsigned width, unsigned height);
    ~ImageProcessor();

    //1 byte per pixel, row-major order
    void setImage(const unsigned char *grayscale, unsigned width, unsigned height);
    unsigned char *getResult(unsigned &width, unsigned &height);

    //?
    void preprocess(unsigned &resultSize);

    //Performs cross correlation on the working image using the specified kernel input.
    //Zero-pads working image if pad is true
    void crossCorrelate(const float *kernel, unsigned kw, unsigned kh, bool pad);

    //returns width=(maxThreshold-minThreshold) for exactly n points above a threshold
    int threshold(unsigned n);

    Point *sortPoints(unsigned &nPoints);
    struct Point *scalePoints(Point *to);
    struct Point calcDisplacement(Point *from);

private:
    //working image
    unsigned width;
    unsigned height;
    unsigned char *data;

    //pixel points
    unsigned numPoints;
    struct Point *points;

};

#endif // IMAGEPROCESSOR_HPP
