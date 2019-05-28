#include "GeoConv.h"
#include <qdebug.h>


/**< calcutale distance between two geografic coordinates
    lonA, latA - coordinates of the first point (in) [deg]
    lonB, latB - coordinates of the second point (in) [deg]
    returned value - calculated distance [m] */
float SarViewer::GeoConv::distGeo(float latA, float lonA, float latB, float lonB) {
    float angle;

    angle = cos(latA*DEG2RAD)*cos(latB*DEG2RAD)*cos((lonB-lonA)*DEG2RAD) + sin(latA*DEG2RAD)*sin(latB*DEG2RAD);
    return EARTH_RADIUS * acos(angle);
}


float SarViewer::GeoConv::distCart(float x1, float y1, float x2, float y2) {
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}


float SarViewer::GeoConv::distCart(float x1, float y1, float h1, float x2, float y2, float h2) {
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (h1-h2)*(h1-h2));
}


/**< calculate x and y distance between two geografic coordinates (with respect to first coordinate)
    latA, lonA - coordinates of the first point [deg]
    lonB, latB - coordinates of the second point [deg]
    y, x       - distances from first to second coordinage [m] */
void SarViewer::GeoConv::geoPos2xy(float latA, float lonA, float latB, float lonB, float *y, float *x) {
    float lon_diff, lat_diff;

    if (latA < 0.0) latA = latA + 360.0;
    if (latB < 0.0) latB = latB + 360.0;
    if (lonA < 0.0) lonA = lonA + 360.0;
    if (lonB < 0.0) lonB = lonB + 360.0;

    lon_diff = lonB - lonA; if (lon_diff > 180.0) lon_diff = 360.0 - lon_diff;
    lat_diff = latB - latA; if (lat_diff > 180.0) lat_diff = 360.0 - lat_diff;

    *x = EARTH_RADIUS*lon_diff*DEG2RAD*cos(0.5*(latA+latB)*DEG2RAD);
    *y = EARTH_RADIUS*lat_diff*DEG2RAD;
}


/**< calculate geografic position based on initial coordinate and x and y distances
    latA, lonA - coordinates of the initial point (in) [deg]
    y, x       - distance in y and x direction (in) [m]
    latB, lonB - coordinates of the final point (out) [deg] */
void SarViewer::GeoConv::xy2geoPos(float latA, float lonA, float y, float x, float *latB, float *lonB) {
    if (latA < 0) latA = latA + 360.0;
    if (lonA < 0) lonA = lonA + 360.0;

    *latB = latA + y/EARTH_RADIUS*RAD2DEG;
    *lonB = lonA + x/EARTH_RADIUS*RAD2DEG/cos(0.5*(latA+(*latB))*DEG2RAD);
}


float SarViewer::GeoConv::exactGeoPos2Deg(float deg, float min, float sec) {
    return deg + min/60.0 + sec/3600.0;
}


/**< calculate x and y distance between two geografic coordinates (with respect to first coordinate)
    latA, lonA - coordinates of the first point [deg]
    lonB, latB - coordinates of the second point [deg]
    y, x       - distances from first to second coordinage [m] */
void SarViewer::GeoConv::mapGeoPos2xy(float latA, float lonA, float latB, float lonB, float *y, float *x) {
    float lon_diff, lat_diff;

    //    if (latA < 0.0) latA = latA + 360.0;
    //    if (latB < 0.0) latB = latB + 360.0;
    //    if (lonA < 0.0) lonA = lonA + 360.0;
    //    if (lonB < 0.0) lonB = lonB + 360.0;

    lon_diff = lonB - lonA; if (lon_diff > 180.0) lon_diff = 360.0 - lon_diff;
    lat_diff = latB - latA; if (lat_diff > 180.0) lat_diff = 360.0 - lat_diff;

    *x = EARTH_RADIUS*lon_diff*DEG2RAD*cos(0.5*(latA+latB)*DEG2RAD);
    *y = EARTH_RADIUS*lat_diff*DEG2RAD;
}


void SarViewer::GeoConv::mapGeoPos2xy(sar_image_line_header *headers, float *Z, int *size, float N0, float E0, float h_ref) {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";
    float lon_diff, lat_diff;

    for(int cnt = 0; cnt < (*size); cnt++) {
        lon_diff = headers[cnt].lon_plat - E0;
        if(lon_diff > 180.0)
            lon_diff = 360.0 - lon_diff;

        lat_diff = headers[cnt].lat_plat - N0;
        if(lat_diff > 180.0)
            lat_diff = 360.0 - lat_diff;

        headers[cnt].lat_plat = EARTH_RADIUS*lon_diff*DEG2RAD*cos(0.5*(N0+headers[cnt].lat_plat)*DEG2RAD); // lat = x
        headers[cnt].lon_plat = EARTH_RADIUS*lat_diff*DEG2RAD; // lon = y
        Z[cnt] = headers[cnt].alt_ground - h_ref; // alt = z

        // To rads
        headers[cnt].head = headers[cnt].head*M_PI/180;
    }
}



void SarViewer::GeoConv::lla2fe(sar_image_line_header *headers, float *Z, int *size, float N0, float E0, float PSI0, float h_ref, float HREF) {
//    qDebug() << "[" << __FILE__ << __LINE__ << __FUNCTION__ << "() ]";

    PSI0 = PSI0*DEG2RAD; // [rad]
    N0 = N0*DEG2RAD; // [rad]
    E0 = E0*DEG2RAD; // [rad]

    // Earth radii computation
    float RN = EARTH_RADIUS/sqrt(1-(2*F-F*F)*(sin(N0)*sin(N0)));
    float RM = RN*(1-(2*F-F*F))/(1-(2*F-F*F)*(sin(N0)*sin(N0)));

    //  If angle between Flat Earth x-axis and North non null
    //      A = [ cos(PSI0) sin(PSI0)  0; ...
    //           -sin(PSI0) cos(PSI0)  0; ...
    //                0         0      1];

    // TRANSFORMATION ALGORITHM
//    float r_min = headers[(*size)-1].r_min;
//    float cell_size = headers[(*size)-1].cell_size;

    for(int cnt = 0; cnt < (*size); cnt++) {
        float Y = ( DEG2RAD * headers[cnt].lat_plat - N0) / atan(1/RM);
        float X = ( DEG2RAD * headers[cnt].lon_plat - E0) / atan(1/(RN*cos(N0)));
        // z w gore
        Z[cnt] = ((-1.0*(-headers[cnt].alt_ground - HREF)) - h_ref);

        // XYZ(i,:) = A * [Y; X; Z]; %[m]
        // os x na wschod
        headers[cnt].lat_plat = cos(PSI0)*(Y) + sin(PSI0)*(X);
        // os y na polnoc
        headers[cnt].lon_plat = -1.0*(-sin(PSI0)*(Y) + cos(PSI0)*(X) );

        // To rads
        headers[cnt].head = headers[cnt].head*M_PI/180;

//        headers[cnt].r_min = r_min;
//        headers[cnt].cell_size = cell_size;
    }
}
