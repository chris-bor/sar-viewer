/// @file MapViewer.h
/// @brief Provides ploting maps, axis, legend, zooms, track, plane.
/// @author Krzysztof Borowiec

#ifndef MAPVIEWER_H
#define MAPVIEWER_H

#include <qvector.h>
#include "qcustomplot.h"
#include "qmainwindow.h"
#include "Config.h"
#include "Plane.h"
#include "MapTextItem.h"
#include "MapPointItem.h"

class QProgressDialog;
//namespace SarViewer {
//class Plane;
//}

#define BACKGROUND_COLOR        Qt::black
#define BASE_PEN_COLOR          Qt::white
#define TICK_LABELS_COLOR       Qt::white
#define TICKS_COLOR             Qt::white
#define SUBTICKS_COLOR          Qt::white
#define LABELS_COLOR            Qt::white

#define EUROPE      0
#define POLAND      1
#define GERMANY     2
#define AREA        EUROPE

#define NR_BITS 33 // numbers of bists at which is saved name of cities 32 bits + '\0'
#define NR_BITS_AIRPORT_NAMED 30 // numbers of bists at which is saved name of airport 29 bits + '\0'

const int LINE_LENG = 200;
const int TEXT_LINE_LENG = 16*32+2;
const int PATH_LENG = 100;
const int MAX_NR_MAP_DETAILS = 10; // coupled with number of MapDetails: coast, bnd prim, bnd inter, city loca, city, lakes, rivers, airports, airports loca, roads
const int MAX_NR_MAP_TEXT_DETAILS = 2; // coupled with number of MapTextDetails: city_named, airport_named
const int MAX_NR_MAP_CURVES = 7; // coast, bnd prim, bnd inter, lakes, rivers, city, roads
const S_V_MAP_FLOAT NaN = 2.139095040000000e+009; // NaN float

#if S_V_PREC == 1
const int TEXT_OFFSET = 16;
#else
const int TEXT_OFFSET = 25;
#endif

const QString MAPS_PATH = QString("maps");
const QString BOUNDARY_PATH = QString("boundary");
const QString POPULATION_PATH = QString("population");
const QString HYDRO_PATH = QString("hydrography");
const QString TRANS_PATH = QString("transportation");
const QString FILE_EXTENSION = QString("dat");
const QString SLASH = QString("/");

#if AREA == EUROPE
const QString AREA_PATH = QString("Europe");
#elif AREA == POLAND
const QString AREA_PATH = QString("Poland");
#else
const QString AREA_PATH = QString("Germany");
#endif

#if S_V_PREC == 1
const QString COAST_ACCURATE_FILE_NAME = QString("coast_accurate");
const QString PRIMARY_BND_FILE_NAME = QString("bnd_primary_all");
const QString INTERNAL_BND_FILE_NAME = QString("bnd_international_accurate");
const QString CITY_NAMED_FILE_NAME = QString("city_named");
const QString CITY_NAMED_LOCATION_FILE_NAME = QString("city_named_location");
const QString CITY_FILE_NAME = QString("city");
const QString LAKES_FILE_NAME = QString("lake_Perennial_Permanent");
const QString RIVERS_FILE_NAME = QString("river_stream_Perennial_Permanent");
const QString AIRPORT_FILE_NAME = QString("airport_named_location");
const QString AIRPORT_NAMED_FILE_NAME = QString("airport_named");
const QString ROAD_FILE_NAME = QString("road");
#else
const QString COAST_ACCURATE_FILE_NAME = QString("coast_accurate_double");
const QString PRIMARY_BND_FILE_NAME = QString("bnd_primary_all_double");
const QString INTERNAL_BND_FILE_NAME = QString("bnd_international_accurate_double");
const QString CITY_NAMED_FILE_NAME = QString("city_named_double");
const QString CITY_NAMED_LOCATION_FILE_NAME = QString("city_named_location_double");
const QString CITY_FILE_NAME = QString("city_double");
const QString LAKES_FILE_NAME = QString("lake_Perennial_Permanent_double");
const QString RIVERS_FILE_NAME = QString("river_stream_Perennial_Permanent_double");
const QString AIRPORT_FILE_NAME = QString("airport_named_location_double");
const QString AIRPORT_NAMED_FILE_NAME = QString("airport_named_double");
const QString ROAD_FILE_NAME = QString("road_double");
#endif


enum MapDetails { // All together
    COAST = 0,
    PRIMARY_BND,
    INTERNAL_BND,
    CITY,
    LAKES,
    RIVERS,
    ROAD,
    AIRPORT,
    AIRPORT_NAMED_LOCATION,
    CITY_NAMED_LOCATION
};


enum MapCurvesNrs {
    COAST_CURVE = 0,
    PRIMARY_BND_CURVE,
    INTERNAL_BND_CURVE,
    CITY_CURVE,
    LAKES_CURVE,
    RIVERS_CURVE,
    ROAD_CURVE
};

enum MapTextDetails { // text map details fg. city named
    CITY_NAMED = 0,
    AIRPORT_NAMED
};

enum MapPointsDetails {
    AIRPORT_POINTS
};


/* 2 pointes struct */
typedef struct TwoGeoCoordType_ {
    S_V_MAP_FLOAT latitude;
    S_V_MAP_FLOAT longitude;
} TwoGeoCoordType;

typedef struct TwoCartCoordType_ {
    S_V_MAP_FLOAT x;
    S_V_MAP_FLOAT y;
} TwoCartCoordType;

typedef struct SpecialPoint_ {
    S_V_FLOAT x;
    S_V_FLOAT y;
    S_V_FLOAT h;
    S_V_FLOAT lat;
    S_V_FLOAT lon;
    QString name;
} SpecialPoint;


typedef MapDetails MapDetailsType;
typedef MapTextDetails MapTextDetailsType;
typedef MapPointsDetails MapPointsDetailsType;
typedef QVector <TwoGeoCoordType> TwoGeoCoordVectType;
typedef QVector <TwoGeoCoordType>::iterator TwoGeoCoordVectIter;
typedef QVector <TwoCartCoordType> TwoCartCoordVectType;
typedef QVector<QString> TextVectType;

namespace SarViewer {
    class MapViewer : public QCustomPlot {
        Q_OBJECT
    public :
        MapViewer(QMainWindow *parent = 0);
        ~MapViewer();
        int manage();
        int plotMap();
        Plane *getPlane() const { return plane_; }

    public slots :
        void setMapDetailsVisib(int map_detail);
        void legendVisible();

    private slots :
        void selectionChanged(); // To connect opposite axis to the same reaction
        void mousePress(); // On axis, label,
        void mouseWheel(); // Manages zoom
        void contextMenuRequest(QPoint pos); // Manages context menu request
        void moveLegend(); // Moves legend
        void keepAspeRatToggled(bool val);

    private :
        int readMap(const QString &fileName, char **fileBuff);
        int getNextLine(char *fileBuff, char *line);
        int getNextTextLine(char *fileBuff, char *line);
        int parseLine2Coord(char *line, TwoGeoCoordVectType &mapDetailCoord, int lineNr, MapDetailsType detailType);
        int parseTextLine(char *line, int lineNr, MapTextDetailsType detail_type);
        void clearMapDetailCoord(MapDetailsType detail_type) { mapDetailCoord[detail_type].clear(); }
        bool getWasCalculated(MapDetailsType detail_type) { return this->wasCalculated[detail_type]; }
        void setWasCalculated(MapDetailsType detail_type, bool value) { this->wasCalculated[detail_type] = value; }
        int manageLineMap(MapDetailsType detail_type);
        int manageTextMap(MapTextDetailsType detail_type);
        int plotLineMap(MapDetailsType detail_type, int subtype, const char *typeStr);
        int plotTextMap(MapTextDetails text_type, MapDetailsType detail_type, const char *typeStr);
        int plotPointMap(MapDetailsType detail_type, MapPointsDetailsType point_type, const char *typeStr);
        void initVisibleMapDeta();
        void manageLayers();

        QMainWindow *parent_;
        QString PATHS[MAX_NR_MAP_DETAILS]; // stores paths to files witch geographical coordinates
        QString PATHS_TEXT[MAX_NR_MAP_TEXT_DETAILS]; // stores paths to text files
        TwoGeoCoordVectType mapDetailCoord[MAX_NR_MAP_DETAILS]; // for storing geografical data
        TwoCartCoordVectType mapDetailCartCoord[MAX_NR_MAP_DETAILS]; // for storing cartesian data
        TextVectType mapTextDetails[MAX_NR_MAP_TEXT_DETAILS]; // stores text details such as names of cities etc.
        QColor mapColors[ MAX_NR_MAP_DETAILS ];
        QColor mapTextColors[ MAX_NR_MAP_TEXT_DETAILS ];
        int map_details_width[ MAX_NR_MAP_DETAILS ];
        SpecialPoint *startPosition;
        bool wasReaded[ MAX_NR_MAP_DETAILS ];
        bool wasTextReaded[ MAX_NR_MAP_DETAILS ];
        bool wasCalculated[ MAX_NR_MAP_DETAILS ];
        bool wasTextCalculated[ MAX_NR_MAP_DETAILS ];// TO CHYBA NIE BEDZIE POTRZEBNE
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
        QList<QCPCurve *> mapCurvesList_; // The list with curves such as rivers, roads etc.
        QList<MapTextItem *> mapTextList_; // The list with points and names such as cities
        QList<MapPointItem *> mapPointsList_; // The list with points fg. airports
        QStringList mapCurveNames_;
        QStringList mapTextNames_; // TODO inicjalizacja i wykorzystanie
        QProgressDialog *progBarManage_;
        Plane *plane_;
    };
} // namespace SarViewer
#endif // MAPVIEWER_H
