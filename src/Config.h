/// @file Config.h
/// @brief configuration file
/// @author Krzysztof Borowiec

#ifndef CONFIG_H
#define CONFIG_H

#include <QDebug>
#include <qobject.h>
#include "Line.h"
#include "LocalSocketReceiverThread.h"

// Definitions
#define NR_FLAG_BITS                8

#define MIN_NR_LINES_TO_DRAW        1000    // Minimum number of lines after which drawer thread gets event to draw received lines
#define MIN_MIN_NR_LINES_TO_DRAW    50
#define MAX_MIN_NR_LINES_TO_DRAW    5000

#define SHIFT_VAL                   200     // 250 value of shift between two plots of pixmap [ pix ]
#define MIN_SHIFT_VAL               MIN_MIN_NR_LINES_TO_DRAW
#define MAX_SHIFT_VAL               5000 // It depends on min_nr_lines_to_draw

#define MIN_MAX_X_RANGE_MAP         10 // [m] range
#define MAX_MAX_X_RANGE_MAP         1e6 // [m] range
#define MIN_MIN_X_RANGE_MAP         -1e6 // [m] range
#define MAX_MIN_X_RANGE_MAP         -10 // [m] range
#define MIN_MAX_Y_RANGE_MAP         10 // [m] range
#define MAX_MAX_Y_RANGE_MAP         1e6 // [m] range
#define MIN_MIN_Y_RANGE_MAP         -1e6 // [m] range
#define MAX_MIN_Y_RANGE_MAP         -10 // [m] range
#define MAX_X_RANGE_MAP             5000 // Default max x range on the map
#define MIN_X_RANGE_MAP             -50000 // Default max x range on the map
#define MAX_Y_RANGE_MAP             5000 // Default max y range on the map
#define MIN_Y_RANGE_MAP             -5000 // Default max y range on the map

#define NR_TRACK_POINTS_DEF         100 // Default number of the trajectory pointes
#define MIN_NR_TRACK_POINTS         0 // All points
#define MAX_NR_TRACK_POINTS         1e6 // Maximal value of the track points

#ifdef __unix__
#define S_V_STR_CAT wcscat
#else
#define S_V_STR_CAT strcat
#endif

#define S_V_PREC 1 /**< sar viewer precision (1 - float, 2 - double) */
//#define S_V_PREC 2 /**< sar viewer precision (1 - float, 2 - double) */
#if S_V_PREC == 1
typedef float S_V_FLOAT;
#define SCANF_PREC "%f"
typedef float S_V_MAP_FLOAT;
#define SCANF_MAP_PREC "%f"
#define SCANF_MAP_TWO_COORD_PERC "%f %f"
#else
typedef double S_V_FLOAT;
#define SCANF_PREC "%lf"
typedef double S_V_MAP_FLOAT;
#define  SCANF_MAP_PREC "%lf"
#define SCANF_MAP_TWO_COORD_PERC "%lf %lf"
#endif

// Constants
const QString DEF_APP_NAME = "SAR viewer";
const QString SAR_VIEWER_VERION_MAJOR = "1";
const QString SAR_VIEWER_VERION_MINOR = "0.70";

const int WINDOW_WIDTH_DEF = 750;
const int WINDOW_HEIGHT_DEF = 500;

const int REPLAY_TIMER_VAL = 250; // time between two plotting of pixmap [ ms ]
const int NR_HIDDEN = 0; // number of hidden translations, not viewed

const QString DEF_CONFIG_FILE_NAME = QString("viewer_config.ini"); // configuration file name

/** Kind of receiving data */
enum KindRecvData {
    /*BinaryFile = 0,*/
    UDP = 0/*,
    NamedPipe*/
};

/** DockWidgets */
enum DockWidgets {
    Stream0 = 0,
    Stream1 = 1,
    NrStreams = 2
};

/** Plotting margins */
enum Margin {
    MarginLeft = 80,
    MarginRight = 20,
    MarginTop = 50,
    MarginBottom = 20,
    ColorbarWidth = 50,
    ColorbarMarginLeft = 5,
    ColorbarMarginRight = 20
};

/** Numbers of ticks on axis for data streams */
enum Ticks {
    NrXTicks = 10,
    NrYTicks = 10,
    NrYTicksColorbar = 10
};


/** geographical position */
#pragma pack(1)
typedef struct GeoPos_ {
    float lat;                          /**< latitude */
    float lon;                          /**< longitude */
} GeoPos;
#pragma pack()

/** configuration parameters numbers */
enum par_type_viewer {
    min_nr_lines_to_draw,
    shift_val,
    min_x_range_map,
    max_x_range_map,
    min_y_range_map,
    max_y_range_map,
    start_pos_map_lat,
    start_pos_map_lon,
    nr_track_points,
    nr_lines_per_screen,
    nr_lin_to_ref_str_scan,
    nr_ref_str_scan,
    show_colorbar_0,
    show_colorbar_1,
    show_coast,
    show_primary_boundary,
    show_internal_boundary,
    show_city_names,
    show_city,
    show_lakes,
    show_rivers,
    show_airport,
    show_airport_names,
    show_road
};


/** structure with program parameters */
#pragma pack(1)
typedef struct ViewerConfig_ {
    uint min_nr_lines_to_draw;          /**< minimum number of lines after which receiver sends signal to draw new data */
    uint shift_val;                     /**< number of lines constitute one shift */
    int min_x_range_map;                /**< minimum horizontal range on the map [m] */
    int max_x_range_map;                /**< maximum horizontal range on the map [m] */
    int min_y_range_map;                /**< minimum vertical range on the map [m] */
    int max_y_range_map;                /**< maximum vertical range on the map [m] */
    GeoPos start_pos_map;               /**< start geographical position on the map [deg'min's] */
    int nr_track_points;                /**< number of the trajectory points ( 0 means all points are stored ) */
    int nr_lines_per_screen;            /**< number of lines per screen */
    uint nr_lin_to_ref_str_scan;        /**< number of lines after which strip scan will be refreshed */
    uint nr_ref_str_scan;               /**< number stored refreshments strip scans */
    bool showColorbar[NrStreams];                 /**< define if show the color bar for stream ([0]-0 [1]-1) */
    bool showCoast;
    bool showPrimaryBnd;
    bool showInternalBnd;
    bool showCityNamed;
    bool showCity;
    bool showLakes;
    bool showRivers;
    bool showAirport;
    bool showAirportNamed;
    bool showRoad;
} ViewerConfig;
#pragma pack()

extern ViewerConfig viewer_config;
#define NR_LINES_PER_SCREEN         viewer_config.nr_lines_per_screen

const int WIDGET_WIDTH_DEF = SarViewer::NR_PIX_PER_LINE;
#define WIDGET_HEIGHT_DEF           viewer_config.nr_lines_per_screen

#define MIN_NR_LIN_TO_REF_STR_SCAN      0
#define NR_LIN_TO_REF_STR_SCAN          2000
#define MAX_NR_LIN_TO_REF_STR_SCAN      MAX_MIN_NR_LINES_TO_DRAW

#define MIN_REF_STR_SCAN                0
#define NR_REF_STR_SCAN                 10
#define MAX_REF_STR_SCAN                100

#define SCREEN_X_RESOLUTION             1366 // 1920
#define SCREEN_Y_RESOLUTION             768 // 1080


/** structure describing strip scan image boundary */
#pragma pack(1)
typedef struct StripScanBndType_ {
    S_V_MAP_FLOAT x_min;
    S_V_MAP_FLOAT y_min;
    S_V_MAP_FLOAT x_max;
    S_V_MAP_FLOAT y_max;
    S_V_MAP_FLOAT x_pix_min;
    S_V_MAP_FLOAT y_pix_min;
    S_V_MAP_FLOAT x_pix_max;
    S_V_MAP_FLOAT y_pix_max;
    uint width_pix;
    uint height_pix;
} StripScanBndType;
#pragma pack()

enum Altitude {
    Zero,
    AltGround,
    AltSea
};

#define ALTITUDE_TYPE                   AltGround   // Defines type of ALTITUDE at whole program
#define H_REFF                          208         // Difference between ground altitude and sea altitude

#endif // CONFIG_H
