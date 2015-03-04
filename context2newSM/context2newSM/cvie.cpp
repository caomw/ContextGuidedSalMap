/*************************************************************
安全な画素値取得を行う関数cvImageElem

How to use
1．このﾌｧｲﾙをｲﾝｸﾙｰﾄﾞする #include "cvie.cpp"
2．
 関数 cvImageElem<IPL_DEPTH_8U>() の返り値は unsigned char になります．
const unsigned char r = cvImageElem<IPL_DEPTH_8U>(image1, x, y, 0); // 0番目のチャンネル，つまり赤R
const unsigned char g = cvImageElem<IPL_DEPTH_8U>(image1, x, y, 1); // 1番目のチャンネル，つまり緑G
const unsigned char b = cvImageElem<IPL_DEPTH_8U>(image1, x, y, 2); // 2番目のチャンネル，つまり青B

 関数 cvImageElem<IPL_DEPTH_64F>() の返り値は double になります．
*const double q = cvImageElem<IPL_DEPTH_64F>(image2, x, y, 0);
const double w = cvImageElem<IPL_DEPTH_64F>(image2, x, y, 1);

// 値の代入も同じようにできます
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
    assert((0 <= x) && (x < cvImage->width) && "cvImageElem(): x座標が範囲外です");
    assert((0 <= y) && (y < cvImage->height) && "cvImageElem(): y座標が範囲外です");
    assert((0 <= channel) && (channel < cvImage->nChannels) && "cvImageElem(): channelが範囲外です");
    assert(IPL_DEPTH == cvImage->depth && "cvImageElem(): 型が一致しません");

    const int channels = cvImage->nChannels;

    typedef typename detail::CvImageElemTypeTraits<IPL_DEPTH>::ImageElemType ImageElemType;

    return reinterpret_cast<ImageElemType*>(cvImage->imageData + cvImage->widthStep * y)[x * channels + (channels - 1 - channel)];
}


#endif
