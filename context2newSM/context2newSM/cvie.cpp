/*************************************************************
���S�ȉ�f�l�擾���s���֐�cvImageElem

How to use
1�D����̧�ق�ݸٰ�ނ��� #include "cvie.cpp"
2�D
 �֐� cvImageElem<IPL_DEPTH_8U>() �̕Ԃ�l�� unsigned char �ɂȂ�܂��D
const unsigned char r = cvImageElem<IPL_DEPTH_8U>(image1, x, y, 0); // 0�Ԗڂ̃`�����l���C�܂��R
const unsigned char g = cvImageElem<IPL_DEPTH_8U>(image1, x, y, 1); // 1�Ԗڂ̃`�����l���C�܂��G
const unsigned char b = cvImageElem<IPL_DEPTH_8U>(image1, x, y, 2); // 2�Ԗڂ̃`�����l���C�܂��B

 �֐� cvImageElem<IPL_DEPTH_64F>() �̕Ԃ�l�� double �ɂȂ�܂��D
*const double q = cvImageElem<IPL_DEPTH_64F>(image2, x, y, 0);
const double w = cvImageElem<IPL_DEPTH_64F>(image2, x, y, 1);

// �l�̑���������悤�ɂł��܂�
cvImageElem<IPL_DEPTH_8U>(image1, x, y, 0) = 255;
cvImageElem<IPL_DEPTH_8U>(image1, x, y, 1) = 255;
cvImageElem<IPL_DEPTH_8U>(image1, x, y, 2) = 255;

cvImageElem<IPL_DEPTH_64F>(image2, x, y, 0) = 3.14 * x * y;
cvImageElem<IPL_DEPTH_64F>(image2, x, y, 1) = (2.71 * x) / y;

*************************************************************/
#ifndef CVIE_HPP_20081011
#define CVIE_HPP_20081011

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif


#include <cassert>
#include <cxtypes.h>


namespace detail {
    template <int> struct CvImageElemTypeTraits;
    template <>    struct CvImageElemTypeTraits<IPL_DEPTH_1U>  { typedef bool           ImageElemType; };
    template <>    struct CvImageElemTypeTraits<IPL_DEPTH_8U>  { typedef unsigned char  ImageElemType; };
    template <>    struct CvImageElemTypeTraits<IPL_DEPTH_8S>  { typedef char           ImageElemType; };
    template <>    struct CvImageElemTypeTraits<IPL_DEPTH_16U> { typedef unsigned short ImageElemType; };
    template <>    struct CvImageElemTypeTraits<IPL_DEPTH_16S> { typedef short          ImageElemType; };
    template <>    struct CvImageElemTypeTraits<IPL_DEPTH_32S> { typedef int            ImageElemType; };
    template <>    struct CvImageElemTypeTraits<IPL_DEPTH_32F> { typedef float          ImageElemType; };
    template <>    struct CvImageElemTypeTraits<IPL_DEPTH_64F> { typedef double         ImageElemType; };
}


template <int IPL_DEPTH>
inline typename detail::CvImageElemTypeTraits<IPL_DEPTH>::ImageElemType&
cvImageElem(const IplImage* cvImage, int x, int y, int channel)
{
    assert((0 <= x) && (x < cvImage->width) && "cvImageElem(): x���W���͈͊O�ł�");
    assert((0 <= y) && (y < cvImage->height) && "cvImageElem(): y���W���͈͊O�ł�");
    assert((0 <= channel) && (channel < cvImage->nChannels) && "cvImageElem(): channel���͈͊O�ł�");
    assert(IPL_DEPTH == cvImage->depth && "cvImageElem(): �^����v���܂���");

    const int channels = cvImage->nChannels;

    typedef typename detail::CvImageElemTypeTraits<IPL_DEPTH>::ImageElemType ImageElemType;

    return reinterpret_cast<ImageElemType*>(cvImage->imageData + cvImage->widthStep * y)[x * channels + (channels - 1 - channel)];
}


#endif
