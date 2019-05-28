/// @file GeoConv.h
/// @brief Geographic conversions
/// @author Krzysztof Borowiec
///	@version 1.0

#ifndef GEOCONV_H
#define GEOCONV_H

#include <cmath>
#include "SarImageDataBuffer.h"

const float EARTH_RADIUS = 6378135.0; // m
const float RAD2DEG   = (180.0/M_PI);
const float DEG2RAD   = (M_PI/180.0);
const float F = 1.0/298.257223563; // Earth flattening
//const float KNOT2MPS  = 0.514444444;
//const float MPS2KNOT  = (1/0.514444444);
//const float FOOT2M    = 0.3048;
//const float M2FOOT    = (1/0.3048);

namespace SarViewer {
    class GeoConv {
    public :
        static float distGeo(float latA, float lonA, float latB, float lonB);
        static float distCart(float x1, float y1, float x2, float y2);
        static float distCart(float x1, float y1, float h1, float x2, float y2, float h2);
        static void geoPos2xy(float latA, float lonA, float latB, float lonB, float *y, float *x);
        static void xy2geoPos(float latA, float lonA, float y, float x, float *latB, float *lonB);
        static float exactGeoPos2Deg(float deg, float min, float sec);
        static void mapGeoPos2xy(float latA, float lonA, float latB, float lonB, float *y, float *x);
        static void mapGeoPos2xy(sar_image_line_header *headers, float *Z, int *size, float N0, float E0, float h_ref);
        static void lla2fe(sar_image_line_header *headers, float *Z, int *size, float N0, float E0, float PSI0, float h_ref, float HREF=0);
    };
} // namespace SarViewer

#endif // GEOCONV_H
