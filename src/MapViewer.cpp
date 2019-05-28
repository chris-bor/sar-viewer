#include <QtGui>
#include "MapViewer.h"
#include "Debugging.h"
#include "GeoConv.h"
#include "ConfigParser.h"

extern ViewerConfig viewer_config;

SarViewer::MapViewer::MapViewer(QMainWindow *parent) :
    QCustomPlot(parent), parent_(parent), startPosition(NULL),
    progBarManage_(NULL), plane_(NULL) {
#if DEBUG_CONST
    qDebug() << "CONSTRUCTOR OF MapViewer\n";
#endif

    // init start position
    if(!startPosition) {
        startPosition = new SpecialPoint;
    }
    startPosition->lat = viewer_config.start_pos_map.lat;
    startPosition->lon = viewer_config.start_pos_map.lon;
    startPosition->x = 0;
    startPosition->y = 0;

    for (int k = 0; k < MAX_NR_MAP_DETAILS; k++ ) {
        wasReaded[k] = false;
        wasCalculated[k] = false;
    }

    for (int k = 0; k < MAX_NR_MAP_TEXT_DETAILS; k++ ) {
        wasTextReaded[k] = false;
        wasTextCalculated[k] = false;
    }

    // init colors
    mapColors[COAST] = QColor( 0, 0, 255 );
    mapColors[PRIMARY_BND] = QColor( 0, 49, 83 );
    mapColors[INTERNAL_BND] = QColor( 0, 127, 255 );
    mapColors[CITY] = QColor( Qt::yellow );
    mapColors[LAKES] = QColor( 51, 0, 204 );
    mapColors[RIVERS] = QColor( 48, 213, 200 );
    mapColors[AIRPORT] = QColor( 128, 0, 128 );
    mapColors[ROAD] = QColor( 0, 102, 51 );
    mapTextColors[CITY_NAMED] = QColor( Qt::red );
    mapTextColors[AIRPORT_NAMED] = QColor( 128, 0, 128 );

    // init width of lines on the map
    for(int k = 0; k < MAX_NR_MAP_DETAILS; k++ )
        map_details_width[k] = 1;

    showCoast = viewer_config.showCoast;
    showPrimaryBnd = viewer_config.showPrimaryBnd;
    showInternalBnd = viewer_config.showInternalBnd;
    showCityNamed = viewer_config.showCityNamed;
    showCity = viewer_config.showCity;
    showLakes = viewer_config.showLakes;
    showRivers = viewer_config.showRivers;
    showAirport = viewer_config.showAirport;
    showAirportNamed = viewer_config.showAirportNamed;
    showRoad = viewer_config.showRoad;

    // initialize paths
    // COAST
    PATHS[COAST] = MAPS_PATH + SLASH + AREA_PATH + SLASH + BOUNDARY_PATH + SLASH + COAST_ACCURATE_FILE_NAME + "." + FILE_EXTENSION;
    // PRIMARY BOUNDARY
    PATHS[PRIMARY_BND] = MAPS_PATH + SLASH + AREA_PATH + SLASH + BOUNDARY_PATH + SLASH + PRIMARY_BND_FILE_NAME + "." + FILE_EXTENSION;
    // INTERRNAL BOUNDARY
    PATHS[INTERNAL_BND] = MAPS_PATH + SLASH + AREA_PATH + SLASH + BOUNDARY_PATH + SLASH + INTERNAL_BND_FILE_NAME + "." + FILE_EXTENSION;
    // CITY NAMES
    PATHS_TEXT[CITY_NAMED] = MAPS_PATH + SLASH + AREA_PATH + SLASH + POPULATION_PATH + SLASH + CITY_NAMED_FILE_NAME + "." + FILE_EXTENSION;
    // CITY NAMED LOCATION
    PATHS[CITY_NAMED_LOCATION] = MAPS_PATH + SLASH + AREA_PATH + SLASH + POPULATION_PATH + SLASH + CITY_NAMED_LOCATION_FILE_NAME + "." + FILE_EXTENSION;
    // CITY
    PATHS[CITY] = MAPS_PATH + SLASH + AREA_PATH + SLASH + POPULATION_PATH + SLASH + CITY_FILE_NAME + "." + FILE_EXTENSION;
    // LAKES
    PATHS[LAKES] = MAPS_PATH + SLASH + AREA_PATH + SLASH + HYDRO_PATH + SLASH + LAKES_FILE_NAME + "." + FILE_EXTENSION;
    // RIVERS
    PATHS[RIVERS] = MAPS_PATH + SLASH + AREA_PATH + SLASH + HYDRO_PATH + SLASH + RIVERS_FILE_NAME + "." + FILE_EXTENSION;
    // AIRPORT
    PATHS[AIRPORT] = MAPS_PATH + SLASH + AREA_PATH + SLASH + TRANS_PATH + SLASH + AIRPORT_FILE_NAME + "." + FILE_EXTENSION;
    // AIRPORT NAMED
    PATHS_TEXT[AIRPORT_NAMED] = MAPS_PATH + SLASH + AREA_PATH + SLASH + TRANS_PATH + SLASH + AIRPORT_NAMED_FILE_NAME + "." + FILE_EXTENSION;
    // ROAD
    PATHS[ROAD] = MAPS_PATH + SLASH + AREA_PATH + SLASH + TRANS_PATH + SLASH + ROAD_FILE_NAME + "." + FILE_EXTENSION;

    mapCurveNames_ << "coast" << "primary boundary" << "internal boundary" << "cities" << "lakes"
                   << "rivers" << "roads";
    mapTextNames_ << "city name" << "airport name";

    // Set available interactions
    setInteractions(QCustomPlot::iRangeDrag | QCustomPlot::iRangeZoom | QCustomPlot::iSelectAxes |
                    QCustomPlot::iSelectLegend | QCustomPlot::iSelectPlottables | QCustomPlot::iSelectTitle);
    setRangeDrag(Qt::Horizontal|Qt::Vertical);
    setRangeZoom(Qt::Horizontal|Qt::Vertical);
    xAxis->setRange(viewer_config.min_x_range_map, viewer_config.max_x_range_map);
    yAxis->setRange(viewer_config.min_y_range_map, viewer_config.max_y_range_map);
    setupFullAxesBox();
    setTitle("");
    xAxis->setLabel("X [ m ]");
    yAxis->setLabel("Y [ m ]");
    xAxis->setTickLabelColor(TICK_LABELS_COLOR);
    yAxis->setTickLabelColor(TICK_LABELS_COLOR);
    xAxis->setLabelColor(LABELS_COLOR);
    yAxis->setLabelColor(LABELS_COLOR);
    QPen pen = xAxis->basePen();
    pen.setColor(BASE_PEN_COLOR);
    xAxis->setBasePen(pen);
    xAxis2->setBasePen(pen);
    yAxis->setBasePen(pen);
    yAxis2->setBasePen(pen);
    legend->setVisible(true);
    pen = xAxis->tickPen();
    pen.setColor(TICKS_COLOR);
    xAxis->setTickPen(pen);
    xAxis2->setTickPen(pen);
    yAxis->setTickPen(pen);
    yAxis2->setTickPen(pen);
    pen = xAxis->subTickPen();
    pen.setColor(SUBTICKS_COLOR);
    xAxis->setSubTickPen(pen);
    xAxis2->setSubTickPen(pen);
    yAxis->setSubTickPen(pen);
    yAxis2->setSubTickPen(pen);

    QFont legend_font = font();
    legend_font.setPointSize(10);
    legend->setFont(legend_font);
    legend->setSelectedFont(legend_font);
    legend->setSelectable(QCPLegend::spItems); // Legend box shall not be selectable, only legend items
    legend->setVisible(false);
    this->setColor(BACKGROUND_COLOR);
    this->setPlottingHint(QCP::phFastPolylines, true);

    // Mange layers
    manageLayers();

    // Create map curves
    for(int cnt = 0; cnt < MAX_NR_MAP_CURVES; cnt++) {
        mapCurvesList_.append( new QCPCurve(this->xAxis, this->yAxis));
        QPen pen = mapCurvesList_.at(cnt)->pen();
        pen.setColor(mapColors[cnt]);
        mapCurvesList_.at(cnt)->setPen(pen);
        mapCurvesList_.at(cnt)->setName(mapCurveNames_.at(cnt));
        mapCurvesList_.at(cnt)->setCurveType(QCPCurve::ctMapCurve);
        this->addPlottable(mapCurvesList_.at(cnt));
    }

    // Create map text details (cities)
    for(int cnt = 0; cnt < MAX_NR_MAP_TEXT_DETAILS; cnt++) {
        mapTextList_.append(new MapTextItem(this->xAxis, this->yAxis));
        QPen pen = mapTextList_.at(cnt)->pen();
        pen.setColor(mapTextColors[cnt]);
        mapTextList_.at(cnt)->setPen(pen);
        mapTextList_.at(cnt)->setName(mapTextNames_.at(cnt));
        this->addPlottable(mapTextList_.at(cnt));
    }
    mapTextList_.at(CITY_NAMED)->setCurveType(QCPCurve::ctPolylines);
    mapTextList_.at(AIRPORT_NAMED)->setCurveType(QCPCurve::ctPoints);

    // Create map point details ( airports )
    mapPointsList_.append(new MapPointItem(this->xAxis, this->yAxis));
    pen = mapPointsList_.at(AIRPORT_POINTS)->pen();
    pen.setColor(mapColors[AIRPORT]);
    mapPointsList_.at(AIRPORT_POINTS)->setPen(pen);
    mapPointsList_.at(AIRPORT_POINTS)->setName("airport");
    this->addPlottable(mapPointsList_.at(AIRPORT_POINTS));

    // Read the map data
    manage();

    // Plot the map
    plotMap();

    // Initialize map details
    initVisibleMapDeta();

    // Create plane_
    plane_ = new SarViewer::Plane(startPosition, this);
    addItem(plane_);

    // Ties some axis selections together (especially opposite axes)
    QObject::connect(this, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    // When an axis is selected, only that direction can be dragged and zoomed:
    QObject::connect(this, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    QObject::connect(this, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));

    // Set the same range at opposite axes
    QObject::connect(xAxis, SIGNAL(rangeChanged(QCPRange)),
                     xAxis2, SLOT(setRange(QCPRange)));
    QObject::connect(yAxis, SIGNAL(rangeChanged(QCPRange)),
                     yAxis2, SLOT(setRange(QCPRange)));
    QObject::connect(xAxis, SIGNAL(rangeChanged(QCPRange)),
                     yAxis, SLOT(setScaleRatioSlot(QCPRange)));

    // setup policy and connect slot for context menu popup
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}


SarViewer::MapViewer::~MapViewer() {
#if DEBUG_DESTR
    qDebug() << "DESTRUCTOR OF MapViewer\n";
#endif
    if(startPosition) {
        delete startPosition;
        startPosition = NULL;
    }
//    if(plane_) {
//        delete plane_;
//        plane_ = 0L;
//    }
}


int SarViewer::MapViewer::manage() {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    if(!progBarManage_) {
        progBarManage_ = new QProgressDialog("Reading and parsing map files", "Cancel", 0, MAX_NR_MAP_DETAILS, parent_);
        progBarManage_->setWindowModality(Qt::WindowModal);
    }
    /* read, parse map detail coordinates */
    for(int k = 0; k < MAX_NR_MAP_DETAILS; k++ ) {
        switch(k) {
        case COAST :
            manageLineMap( COAST );
            break;

        case PRIMARY_BND :
            manageLineMap( PRIMARY_BND );
            break;

        case INTERNAL_BND :
            manageLineMap( INTERNAL_BND );
            break;

        case CITY_NAMED_LOCATION :
            manageTextMap( CITY_NAMED );
            manageLineMap( CITY_NAMED_LOCATION );
            break;

        case CITY :
            manageLineMap( CITY );
            break;

        case LAKES :
            manageLineMap( LAKES );
            break;

        case RIVERS :
            manageLineMap( RIVERS );
            break;

        case AIRPORT :
            manageLineMap( AIRPORT );
            break;

        case AIRPORT_NAMED_LOCATION :
            manageTextMap( AIRPORT_NAMED );
            break;

        case ROAD :
            manageLineMap( ROAD );
            break;

        default:
            break;
        }
        progBarManage_->setValue(k);
    }
    progBarManage_->setValue(MAX_NR_MAP_DETAILS);
    return 0;
}


int SarViewer::MapViewer::plotMap() {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    if(!progBarManage_) {
        progBarManage_ = new QProgressDialog("Reading and parsing map files", "Cancel", 0, MAX_NR_MAP_DETAILS, parent_);
        progBarManage_->setWindowModality(Qt::WindowModal);
    }
    progBarManage_->setLabelText("Plotting map");

    /* read, parse map detail coordinates */
    for(int k = 0; k < MAX_NR_MAP_DETAILS; k++ )
    {
        switch(k) {
        case COAST :
//            if( showCoast )
                plotLineMap( COAST, COAST_CURVE, "Coast" );
            break;

        case PRIMARY_BND :
//            if( showPrimaryBnd )
                plotLineMap( PRIMARY_BND, PRIMARY_BND_CURVE, "Primary boundary" );
            break;

        case INTERNAL_BND :
//            if( showInternalBnd )
                plotLineMap( INTERNAL_BND, INTERNAL_BND_CURVE, "Internal boundary" );
            break;

        case CITY_NAMED_LOCATION :
//            if( showCityNamed )
                plotTextMap( CITY_NAMED, CITY_NAMED_LOCATION, "City named");
            break;

        case CITY :
//            if( showCity )
                plotLineMap( CITY, CITY_CURVE, "City");
            break;

        case LAKES :
//            if( showLakes )
                plotLineMap( LAKES, LAKES_CURVE, "Lakes" );
            break;

        case RIVERS :
//            if( showRivers )
                plotLineMap( RIVERS, RIVERS_CURVE, "Rivers" );
            break;

        case AIRPORT :
//            if( showAirport )
                plotPointMap( AIRPORT, AIRPORT_POINTS, "Airports" );
            break;

        case AIRPORT_NAMED_LOCATION :
//            if( showAirportNamed )
                plotTextMap( AIRPORT_NAMED, AIRPORT, "Airport names");
            break;

        case ROAD :
//            if( showRoad )
                plotLineMap( ROAD, ROAD_CURVE, "Roads" );
            break;

        default:
            break;
        }
        progBarManage_->setValue(k);
    }

    progBarManage_->setValue(MAX_NR_MAP_DETAILS);

    return 0;
}


void SarViewer::MapViewer::setMapDetailsVisib(int map_detail) {
//    qDebug() << __FUNCTION__ << "()";
    if(map_detail < MAX_NR_MAP_CURVES) {
        mapCurvesList_.at(map_detail)->setVisible(!mapCurvesList_.at(map_detail)->visible());
    }

    if(map_detail == AIRPORT_NAMED_LOCATION) {
        mapTextList_.at(AIRPORT_NAMED)->setVisible(!mapTextList_.at(AIRPORT_NAMED)->visible());
    }

    if(map_detail == CITY_NAMED_LOCATION) {
        mapTextList_.at(CITY_NAMED)->setVisible(!mapTextList_.at(CITY_NAMED)->visible());
    }

    if(map_detail == AIRPORT) {
        mapPointsList_.at(AIRPORT_POINTS)->setVisible(!mapPointsList_.at(AIRPORT_POINTS)->visible());
    }
    replot();
}


void SarViewer::MapViewer::legendVisible() {
    this->legend->setVisible(!legend->visible());
    replot();
}


void SarViewer::MapViewer::selectionChanged() {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    if(xAxis->selected().testFlag(QCPAxis::spAxis) || xAxis->selected().testFlag(QCPAxis::spTickLabels) ||
            xAxis2->selected().testFlag(QCPAxis::spAxis) || xAxis2->selected().testFlag(QCPAxis::spTickLabels))
    {
        xAxis2->setSelected(QCPAxis::spAxis|QCPAxis::spTickLabels);
        xAxis->setSelected(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
    if(yAxis->selected().testFlag(QCPAxis::spAxis) || yAxis->selected().testFlag(QCPAxis::spTickLabels) ||
            yAxis2->selected().testFlag(QCPAxis::spAxis) || yAxis2->selected().testFlag(QCPAxis::spTickLabels))
    {
        yAxis2->setSelected(QCPAxis::spAxis|QCPAxis::spTickLabels);
        yAxis->setSelected(QCPAxis::spAxis|QCPAxis::spTickLabels);
    }

    // synchronize selection of the map detail with selection of corresponding legend items:
    for (int i = 0; i < plottableCount(); ++i) {
        QCPAbstractPlottable *plottable = this->plottable(i);
        QCPPlottableLegendItem *item = legend->itemWithPlottable(plottable);
        if (item->selected() || plottable->selected()) {
            item->setSelected(true);
            plottable->setSelected(true);
        }
    }
}


void SarViewer::MapViewer::mousePress() {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    // If an axis is selected, only allow the direction of that axis to be dragged
    // otherwise, both directions may be dragged
    if(xAxis->selected().testFlag(QCPAxis::spAxis))
        setRangeDrag(xAxis->orientation());
    else if(yAxis->selected().testFlag(QCPAxis::spAxis))
        setRangeDrag(yAxis->orientation());
    else
        setRangeDrag(Qt::Horizontal|Qt::Vertical);
}


void SarViewer::MapViewer::mouseWheel() {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    // If an axis is selected, only allow the direction of that axis to be zoomed
    // otherwise, both directions may be zoomed
    if(xAxis->selected().testFlag(QCPAxis::spAxis))
        setRangeZoom(xAxis->orientation());
    else if(yAxis->selected().testFlag(QCPAxis::spAxis))
        setRangeZoom(yAxis->orientation());
    else
        setRangeZoom(Qt::Horizontal|Qt::Vertical);
}


void SarViewer::MapViewer::contextMenuRequest(QPoint pos) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if(legend->selectTestLegend(pos)) // context menu on legend requested
    {
        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)QCPLegend::psTopLeft);
        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)QCPLegend::psTop);
        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)QCPLegend::psTopRight);
        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)QCPLegend::psBottomRight);
        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)QCPLegend::psBottomLeft);
    } else { // general context menu on graphs requested

    }

    menu->popup(mapToGlobal(pos));
}


void SarViewer::MapViewer::moveLegend() {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    if(QAction* contextAction = qobject_cast<QAction*>(sender())) // Make sure this slot is really called by a context menu action, so it carries the data we need
    {
        bool ok;
        int dataInt = contextAction->data().toInt(&ok);
        if(ok)
        {
            legend->setPositionStyle((QCPLegend::PositionStyle)dataInt);
            replot();
        }
    }
}

void SarViewer::MapViewer::keepAspeRatToggled(bool val) {
    qDebug() << __FILE__ << __FUNCTION__ << "()";
    if(val) {
        QObject::connect(xAxis, SIGNAL(rangeChanged(QCPRange)), yAxis, SLOT(setScaleRatioSlot(QCPRange)));
        xRangeChanged();
        replot();
    }
    else {
        QObject::disconnect(xAxis, 0, yAxis, 0);
    }
}


int SarViewer::MapViewer::readMap(const QString &fileName, char **fileBuff) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    FILE *file;
    uint fileLeng;
    size_t cnt;

    if (DEBUG_MAP_VIEWER_READ) {
        qDebug() << "\nLoading file"<< fileName;
    }

    file = fopen(fileName.toAscii(), "rb");
    if (file == NULL) {
        qDebug() << "!!! ERROR: Cannot open file " << fileName << "!!!";
        return -1;
    }

    fseek(file, 0, SEEK_END);
    fileLeng = ftell(file);
    fseek(file, 0, SEEK_SET);

    *fileBuff = (char *)malloc((fileLeng+1)*sizeof(char));

    if ((!(*fileBuff))) {
        qDebug() << "!!! ERROR: Cannot allocate buffer for file (" << fileLeng << "bytes) !!!";
        return -1;
    }

    if (DEBUG_MAP_VIEWER_READ) {
        qDebug() << "File lenght" << fileLeng;
    }

    if ((cnt = fread(*fileBuff, sizeof(char), fileLeng, file)) != fileLeng) {
        qDebug() << "!!! ERROR: Cannot read" << fileLeng << "bytes form file, only" << cnt << "bytes readed !!!";
        return -1;
    }

    *(*fileBuff+cnt) = '\0';

    fclose(file);

    if (DEBUG_MAP_VIEWER_READ) {
        qDebug() << "File" << fileName << "readed successfully";
    }

    return 0;
}


int SarViewer::MapViewer::getNextLine(char *fileBuff, char *line) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    static char *cur_ptr;
    int char_cnt = 0;

    if (fileBuff != NULL)
        cur_ptr = fileBuff;

    while (*cur_ptr != '\n')
    {
        if (*cur_ptr == '\0') /* EOF */
        {
            line[char_cnt] = '\0';
            return 1;
        }
        line[char_cnt++] = *(cur_ptr++);
    }
    cur_ptr++;
    line[char_cnt] = '\0';

    return 0;
}


int SarViewer::MapViewer::getNextTextLine(char *fileBuff, char *line) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    static char *cur_txt_ptr;
    int char_cnt = 0;

    if (fileBuff != NULL)
        cur_txt_ptr = fileBuff;

    while ( *cur_txt_ptr != '\n' )
    {
        if (*cur_txt_ptr == '\0') // EOF
        {
            line[char_cnt] = '\0';
            return 1;
        }
        line[char_cnt++] = *(cur_txt_ptr++);
    }

    cur_txt_ptr++;
    line[char_cnt] = '\0';

    return 0;
}


int SarViewer::MapViewer::parseLine2Coord(char *line, TwoGeoCoordVectType &mapDetailCoord, int lineNr, MapDetailsType detailType) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    char *ptr;
    TwoGeoCoordType tmp_geo_coord;

    ptr = line;

    // skip whitespace characters
    while (*ptr == ' ' || *ptr == '\t')
        ptr++;

    if (*ptr == '\0') // end of line
    {
        return 0;
    }

    /* find first NaN */
    if(*ptr == 'N' && *(ptr+1) == 'a' && *(ptr+2) == 'N' ){
        tmp_geo_coord.latitude = NaN;
        ptr+=3;
    }

    // skip whitespace characters
    while (*ptr == ' ' || *ptr == '\t')
        ptr++;

    /* find second NaN */
    if(*ptr == 'N' && *(ptr+1) == 'a' && *(ptr+2) == 'N' ){
        tmp_geo_coord.longitude = NaN;
        ptr+=3;
        mapDetailCoord.push_back(tmp_geo_coord);
        return 0; /* end of finding NaNs */
    }

    /* read coordinates */
    if (sscanf(ptr, "%e.8", &tmp_geo_coord.latitude) != 1) {
        qDebug() << "!!! ERROR: Coordinates" << QString::fromAscii(ptr) << "not valid !!!";
        return -1;
    }
    ptr+=14;
    // skip whitespace characters
    while (*ptr == ' ' || *ptr == '\t')
        ptr++;

    if (sscanf(ptr, "%e.8", &tmp_geo_coord.longitude) != 1) {
        qDebug() << "!!! ERROR: Coordinates" << QString::fromAscii(ptr) << "not valid !!!";
        return -1;
    }

    mapDetailCoord.push_back(tmp_geo_coord);

    return 0;
}


int SarViewer::MapViewer::parseTextLine(char *line, int lineNr, MapTextDetailsType detail_type) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    char *ptr;
    char tmp_str[NR_BITS];
    S_V_MAP_FLOAT tmp_f;
    char name[NR_BITS];
    bool No_entry;
    bool Space;

    ptr = line;
    No_entry = false;
    Space = false;
    memset(name, 0, NR_BITS);

    // read name
    for(int k=0; k<NR_BITS-1;k++) {
        //memset(tmp_str, 0, NR_BITS);

        // skip whitespace characters
        while (*ptr == ' ' || *ptr == '\t')
            ptr++;

        if (*ptr == '\0') // end of line
        {
            return 0;
        }

        // read name
        if (sscanf(ptr, SCANF_MAP_PREC, &tmp_f) != 1 ) {
            if(detail_type == AIRPORT_NAMED) {
                break; // airport names are shorter 29 no 32
            }
            else {
                qDebug() << "!!! ERROR: Name" << QString::fromAscii(ptr) << "does not valid !!!";
                return -1;
            }
        }

        ptr+=TEXT_OFFSET;

        sprintf(tmp_str, "%c", (int)tmp_f);

        // check if line isn't 'No entry present'
        if( *tmp_str == 'N' ){
            No_entry = true;
        }
        if( *tmp_str == 'o' && No_entry == true){
            mapTextDetails[detail_type].push_back(QString(" "));
            return 0;
        }
/*
        if( (*tmp_str == 0) && (Space == false) ) {
            Space = true;
        }
        else if( (*tmp_str == 0) && (Space == true) ){
            mapTextDetails[detail_type].push_back(name);
            return 0;

        }*/

        strcat(name, tmp_str);
        QString str(name);
        if(k==NR_BITS-2 || (detail_type == AIRPORT_NAMED && k==NR_BITS_AIRPORT_NAMED-2)) {
            mapTextDetails[detail_type].push_back(str);
        }
    }

    return 0;
}


int SarViewer::MapViewer::manageLineMap(MapDetailsType detail_type) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    QString fileName;
    char *fileBuff;
    char line[LINE_LENG];
    int lineNr = 1;

    fileName = PATHS[detail_type];
    if (readMap(fileName, &fileBuff) != 0)
        return -1;

    getNextLine(fileBuff, line);

    if (parseLine2Coord(line, mapDetailCoord[detail_type], lineNr, detail_type) == -1) {
        qDebug() << "!!! ERROR: Cannot parse map file" << fileName << "!!!";
        free(fileBuff);
        return -2;
    }
    lineNr++;

    while (getNextLine(NULL, line) == 0) {
        if (parseLine2Coord(line, mapDetailCoord[detail_type], lineNr, detail_type) == -1) {
            qDebug() << "!!! ERROR: Cannot read map file !!!";
            free(fileBuff);
            return -3;
        }
        lineNr++;
    }

    if (parseLine2Coord(line, mapDetailCoord[detail_type], lineNr, detail_type) == -1) {
        qDebug() << "!!! ERROR: Cannot read map file !!!";
        free(fileBuff);
        return -4;
    }
    free(fileBuff);

    if(DEBUG_MAP_VIEWER_READ)
        qDebug() << "End of reading and parsing map file:" << fileName;

    return 0;
}


int SarViewer::MapViewer::manageTextMap(MapTextDetailsType detail_type) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    QString fileName;
    char *fileBuff;
    char line[TEXT_LINE_LENG];
    int lineNr = 1;

    fileName = PATHS_TEXT[detail_type];
    if (readMap(fileName, &fileBuff) != 0)
        return -1;

    getNextTextLine( fileBuff, line );

    if (parseTextLine(line, lineNr, detail_type) == -1) {
        qDebug() << "!!! ERROR: Cannot parse map file" << fileName << "!!!";
        free(fileBuff);
        return -2;
    }
    lineNr++;

    while (getNextTextLine(NULL, line) == 0 ) {
        if (parseTextLine(line, lineNr, detail_type) == -1) {
            qDebug() << "!!! ERROR: Cannot read map file !!!";
            free(fileBuff);
            return -3;
        }
        lineNr++;
    }
    if (parseTextLine(line, lineNr, detail_type) == -1) {
        qDebug() << "!!! ERROR: Cannot read map file !!!";
        free(fileBuff);
        return -4;
    }
    free(fileBuff);

    if(DEBUG_MAP_VIEWER_READ)
        qDebug() << "End of reading and parsing map file:" << fileName;

    return 0;
}


int SarViewer::MapViewer::plotLineMap(MapDetailsType detail_type, int subtype, const char *typeStr) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    TwoCartCoordType tmpCartCoord;

    /* calculate cartesian coordinates */
    if(!getWasCalculated(detail_type)) {
        for( int k = 0; k < mapDetailCoord[detail_type].size(); k++) {
            if( mapDetailCoord[detail_type][k].latitude != NaN ) {
                SarViewer::GeoConv::mapGeoPos2xy(startPosition->lat, startPosition->lon,
                                        mapDetailCoord[detail_type][k].latitude, mapDetailCoord[detail_type][k].longitude,
                                        &tmpCartCoord.x, &tmpCartCoord.y );
                mapDetailCartCoord[detail_type].push_back(tmpCartCoord);
            }
            else {
//                GeoConv::mapGeoPos2xy(startPosition->lat, startPosition->lon,
//                                        mapDetailCoord[detail_type][k-1].latitude, mapDetailCoord[detail_type][k-1].longitude,
//                                        &tmpCartCoord.x, &tmpCartCoord.y );
                tmpCartCoord.x = NaN;
                tmpCartCoord.y = NaN;
                mapDetailCartCoord[detail_type].push_back(tmpCartCoord);
            }
        }
        setWasCalculated(detail_type, true);
        clearMapDetailCoord( detail_type );

        if (DEBUG_MAP_VIEWER_PLOT) {
            qDebug() << typeStr << " coordinates calculated successfully";
        }
    }

    // Add data to curve:
    if( mapDetailCartCoord[detail_type].size() > 1 ) {
        if(detail_type == COAST || detail_type == PRIMARY_BND || detail_type == INTERNAL_BND ||
           detail_type == LAKES || detail_type == RIVERS || detail_type == ROAD || detail_type == CITY) {
            for( int k = 0; k < (mapDetailCartCoord[subtype].size()); k++) {
                mapCurvesList_.at(subtype)->addData(mapDetailCartCoord[subtype][k].y, mapDetailCartCoord[subtype][k].x);
            }
        }

        if (DEBUG_MAP_VIEWER_PLOT) {
            qDebug("%s ploted successfully\n", typeStr);
        }
    }
    else {
        qDebug("ERROR: %s - could not find map file\n", typeStr);
    }
    return 0;
}


int SarViewer::MapViewer::plotTextMap(MapTextDetails text_type, MapDetailsType detail_type, const char *typeStr) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    TwoCartCoordType tmpCartCoord;

    /* calculate cartesian coordinates */
    if(!getWasCalculated(detail_type)) {

        for( int k = 0; k < mapDetailCoord[detail_type].size(); k++) {
                SarViewer::GeoConv::mapGeoPos2xy(startPosition->lat, startPosition->lon,
                                        mapDetailCoord[detail_type][k].latitude, mapDetailCoord[detail_type][k].longitude,
                                        &tmpCartCoord.x, &tmpCartCoord.y );
                mapDetailCartCoord[detail_type].push_back(tmpCartCoord);
        }
        setWasCalculated(detail_type, true);
        clearMapDetailCoord( detail_type );

        if (DEBUG_MAP_VIEWER_PLOT) {
            qDebug() << typeStr << " coordinates calculated successfully";
        }
    }

    // Add data to MapTextItem:
    if( mapDetailCartCoord[detail_type].size() > 1 ) {
        for( int k = 0; k < (mapDetailCartCoord[detail_type].size()); k++) {
            mapTextList_.at(text_type)->addData(mapDetailCartCoord[detail_type][k].y+TEXT_OFFSET_Y,
                                                mapDetailCartCoord[detail_type][k].x+TEXT_OFFSET_X);
            mapTextList_.at(text_type)->addName(mapTextDetails[text_type][k]);
        }

        if (DEBUG_MAP_VIEWER_PLOT) {
            qDebug("%s ploted successfully\n", typeStr);
        }
    }
    else {
        qDebug("ERROR: %s - could not find map file\n", typeStr);
    }
    return 0;
}


int SarViewer::MapViewer::plotPointMap(MapDetailsType detail_type, MapPointsDetailsType point_type, const char *typeStr) {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    TwoCartCoordType tmpCartCoord;

    /* calculate cartesian coordinates */
    if(!getWasCalculated(detail_type)) {
        for( int k = 0; k < mapDetailCoord[detail_type].size(); k++) {
                SarViewer::GeoConv::mapGeoPos2xy(startPosition->lat, startPosition->lon,
                                        mapDetailCoord[detail_type][k].latitude, mapDetailCoord[detail_type][k].longitude,
                                        &tmpCartCoord.x, &tmpCartCoord.y );
                mapDetailCartCoord[detail_type].push_back(tmpCartCoord);
        }
        setWasCalculated(detail_type, true);
        clearMapDetailCoord( detail_type );

        if (DEBUG_MAP_VIEWER_PLOT) {
            qDebug() << typeStr << " coordinates calculated successfully";
        }
    }

    // Add data to MapTextItem
    if( mapDetailCartCoord[detail_type].size() > 1 ) {
        for( int k = 0; k < (mapDetailCartCoord[detail_type].size()); k++) {
            mapPointsList_.at(point_type)->addData(mapDetailCartCoord[detail_type][k].y, mapDetailCartCoord[detail_type][k].x);
        }

        if (DEBUG_MAP_VIEWER_PLOT) {
            qDebug("%s ploted successfully\n", typeStr);
        }
    }
    else {
        qDebug("ERROR: %s - could not find map file\n", typeStr);
    }
    return 0;
}


void SarViewer::MapViewer::initVisibleMapDeta() {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    showCoast = viewer_config.showCoast;
    showPrimaryBnd = viewer_config.showPrimaryBnd;
    showInternalBnd = viewer_config.showInternalBnd;
    showCityNamed = viewer_config.showCityNamed;
    showCity = viewer_config.showCity;
    showLakes = viewer_config.showLakes;
    showRivers = viewer_config.showRivers;
    showAirport = viewer_config.showAirport;
    showAirportNamed = viewer_config.showAirportNamed;
    showRoad = viewer_config.showRoad;

    // curves    coast, bnd prim, bnd inter, lakes, rivers, city, roads :
    mapCurvesList_.at(COAST_CURVE)->setVisible(showCoast);
    mapCurvesList_.at(PRIMARY_BND_CURVE)->setVisible(showPrimaryBnd);
    mapCurvesList_.at(INTERNAL_BND_CURVE)->setVisible(showInternalBnd);
    mapCurvesList_.at(CITY_CURVE)->setVisible(showCity);
    mapCurvesList_.at(LAKES_CURVE)->setVisible(showLakes);
    mapCurvesList_.at(RIVERS_CURVE)->setVisible(showRivers);
    mapCurvesList_.at(ROAD_CURVE)->setVisible(showRoad);

    // text    city_named, airport_named
    mapTextList_.at(AIRPORT_NAMED)->setVisible(showAirportNamed);
    mapTextList_.at(CITY_NAMED)->setVisible(showCityNamed);

    // points   airports
    mapPointsList_.at(AIRPORT_POINTS)->setVisible(showAirport);

    replot();
}


void SarViewer::MapViewer::manageLayers() {
//    qDebug() << __FILE__ << __FUNCTION__ << "()";
    // Create sar image layer above "grid" layer and beneath "main"
    addLayer("image", this->layer("grid"));
}

