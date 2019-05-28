#include "ConfigParser.h"
#include "Debugging.h"
#include "GeoConv.h"
#include "Line.h"

ViewerConfig viewer_config = { MIN_NR_LINES_TO_DRAW, SHIFT_VAL,
                               MIN_X_RANGE_MAP, MAX_X_RANGE_MAP,
                               MIN_Y_RANGE_MAP, MAX_Y_RANGE_MAP,
                               {0, 0},
                               NR_TRACK_POINTS_DEF,
                               NR_LINES_PER_SCREEN,
                               NR_LIN_TO_REF_STR_SCAN,
                               NR_REF_STR_SCAN,
                               {true, true} /*show_colorbar_0 show_colorbar_1*/,
                               true /*show_coast*/,
                               true /*show_primary_boundary*/,
                               true /*show_internal_boundary*/,
                               true /*show_city_names*/,
                               true /*show_city*/,
                               true /*show_lakes*/,
                               true /*show_rivers*/,
                               true /*show_airport*/,
                               true /*show_airport_names*/,
                               true /*show_road*/
};

static const char pars_tab_viewer[MAX_NR_PARS][MAX_PAR_LEN] = {
    "min_nr_lines_to_draw",
    "shift_val",
    "min_x_range_map",
    "max_x_range_map",
    "min_y_range_map",
    "max_y_range_map",
    "start_lat_pos_map",
    "start_lon_pos_map",
    "nr_track_points",
    "nr_lines_per_screen",
    "nr_lin_to_ref_str_scan",
    "nr_ref_str_scan",
    "show_colorbar_0",
    "show_colorbar_1"
};


SarViewer::ConfigParser::ConfigParser(QObject *parent) : QObject(parent){
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF ConfigParser\n";
#endif
}


SarViewer::ConfigParser::~ConfigParser() {
#if DEBUG_DESTR
        qDebug() << "DESTRUCTOR OF ConfigParser\n";
#endif
}


int SarViewer::ConfigParser::parseConfigFile(const QString &file_name, ViewerConfig *viewer_config) {
    char line[LINE_LEN];
    char file_buf[FILE_LEN];
    int line_nr = 1;

    if (readConfigFile(file_name, file_buf) != 0)
        return -1;

    removeComments(file_buf);

    getNextLine(file_buf, line);
    if (parseLine(line, viewer_config, line_nr) == -1)
    {
        printf("!!! Error reading config file !!!\n");
        return 1;
    }
    line_nr++;

    while (getNextLine(NULL, line) == 0)
    {
        if (parseLine(line, viewer_config, line_nr) == -1)
        {
            printf("!!! Error reading config file !!!\n");
            return 1;
        }
        line_nr++;
    }

    if (parseLine(line, viewer_config, line_nr) == -1)
    {
        printf("!!! Error reading config file !!!\n");
        return 1;
    }

    return 0;
}

void SarViewer::ConfigParser::printf_pars(ViewerConfig *v_c) {
    qDebug(" ");
    qDebug("Viewer parametars:");
    qDebug("min_nr_lines_to_draw        = %d", v_c->min_nr_lines_to_draw);
    qDebug("shift_val                   = %d", v_c->shift_val);
    qDebug("min_x_range_map             = %d", v_c->min_x_range_map);
    qDebug("max_x_range_map             = %d", v_c->max_x_range_map);
    qDebug("min_y_range_map             = %d", v_c->min_y_range_map);
    qDebug("max_y_range_map             = %d", v_c->max_y_range_map);
    qDebug("start_pos_map_lat           = %.2f", v_c->start_pos_map.lat);
    qDebug("start_pos_map_lon           = %.2f", v_c->start_pos_map.lon);
    qDebug("nr_track_points             = %d", v_c->nr_track_points);
    qDebug("nr_lines_per_screen         = %d", v_c->nr_lines_per_screen);
    qDebug("nr_lin_to_ref_str_scan      = %d", v_c->nr_lin_to_ref_str_scan);
    qDebug("nr_ref_str_scan             = %d", v_c->nr_ref_str_scan);
    qDebug(" ");
    qDebug("show_colorbar_0             = %d", v_c->showColorbar[Stream0]);
    qDebug("show_colorbar_1             = %d", v_c->showColorbar[Stream1]);
    qDebug(" ");
}


int SarViewer::ConfigParser::readConfigFile(const QString &file_name, char *file_buf) {
    FILE *file;
    int cnt = 0;

    if (DEBUG_FILE)
            printf("Loading file '%s'\n", file_name.toLocal8Bit().data());

    file = fopen(file_name.toAscii(), "r");
    if (file == NULL)
    {
        printf("!!! Cannot open config file '%s' !!!\n", file_name.toLocal8Bit().data());
        return -1;
    }

    while (fread(file_buf+cnt, sizeof(char), 1, file))
    {
        cnt++;

        if (cnt == FILE_LEN)
        {
            printf("!!! Buffer for config file too short !!!\n");
            return -1;
        }
    }

    file_buf[cnt] = '\0';

    fclose(file);
    if (DEBUG_PARSE)
        printf("\rFile '%s' loaded successfully\n", file_name.toLocal8Bit().data());

    return 0;
}

void SarViewer::ConfigParser::removeComments(char *buf) {
    char *ptr1, *ptr2;
    int comment = 0;

    ptr1 = buf;
    ptr2 = buf;

    while(*ptr1 != '\0')
    {
        if (*ptr1 == COMMENT_CHAR)
            comment = 1;
        if (*ptr1 == '\n')
            comment = 0;

        if (comment == 0)
            *(ptr2++) = *(ptr1++);
        else
            ptr1++;
    }
    *ptr2 = '\0';
}


int SarViewer::ConfigParser::getNextLine(char *file_buf, char *line) {
    static char *cur_ptr;
    int char_cnt = 0;

    if (file_buf != NULL)
        cur_ptr = file_buf;

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


int SarViewer::ConfigParser::parseLine(char *line, ViewerConfig *viewer_config, int line_nr) {
    char *ptr;
    char buf[LINE_LEN];
    int buf_cnt;
    int val_i;
    int val_deg = 0, val_min = 0, val_sec = 0;
//    float val_f;
    //char val_s[STRING_LEN];
    int k;
    int type = -1;

    ptr = line;
    // skip whitespace characters
    while (*ptr == ' ' || *ptr == '\t')
        ptr++;

    if (*ptr == '\0') // empty line
        return 1;

    buf_cnt = 0;

    /* read parameter name */
    while (*ptr != ' ' && *ptr != '\t' && *ptr != '=')
    {
        if (*ptr == '\0')
        {
            printf("!!! Unexpected end of line in config file (line %d, column %d) !!!", line_nr, ptr-line+1);
            return -1;
        }
        buf[buf_cnt++] = *(ptr++);
    }

    /* find parameter name */
    buf[buf_cnt] = '\0';
    for (k = 0; k < MAX_NR_PARS; k++)
    {
        if (strcmp(pars_tab_viewer[k], buf) == 0)
        {
            type = k;
            break;
        }
    }
    if (type == -1)
    {
        printf("!!! Parameter '%s' not recognized !!!\n", buf);
        return -1;
    }

    /* skip whitespace characters */
    while (*ptr == ' ' || *ptr == '\t')
    {
        if (*ptr == '\0')
        {
            printf("!!! Unexpected end of line in config file (line %d, column %d) !!!", line_nr, ptr-line+1);
            return -1;
        }
        ptr++;
    }

    if (*ptr == '\0')
    {
        printf("!!! Unexpected end of line in config file (line %d, column %d) !!!\n", line_nr, ptr-line+1);
        return -1;
    }

    /* read operator */
    if (*ptr != '=')
    {
        printf("!!! Operator '%c' not recognized (line %d, column %d) !!!\n", *ptr, line_nr, ptr-line+1);
        return -1;
    }
    ptr++;

    /* skip whitespace characters */
    while (*ptr == ' ' || *ptr == '\t')
    {
        if (*ptr == '\0')
        {
            printf("!!! Unexpected end of line in config file (line %d, column %d) !!!\n", line_nr, ptr-line+1);
            return -1;
        }
        ptr++;
    }

    if (*ptr == '\0')
    {
        printf("!!! Unexpected end of line in config file (line %d, column %d) !!!\n", line_nr, ptr-line+1);
        return -1;
    }

    /* float parameters */
//    if (type == )
//    {
//        if (sscanf(ptr, "%f", &val_f) != 1)
//        {
//            printf("!!! Parameter value '%s' not valid !!!\n", ptr);
//            return -1;
//        }

//        switch (type)
//        {
//        case max_x_range_map:
//            if (checkFloatPar("max_x_range_map", val_f, MIN_MAX_X_RANGE_MAP, MAX_MAX_X_RANGE_MAP) == 0)
//                viewer_config->max_x_range_map = val_f;
//            else
//                return -1;
//            break;
//        case max_y_range_map:
//            if (checkFloatPar("max_y_range_map", val_f, MIN_MAX_Y_RANGE_MAP, MAX_MAX_Y_RANGE_MAP) == 0)
//                viewer_config->max_y_range_map = val_f;
//            else
//                return -1;
//            break;
//        }
//    }
    /* integer parameters */
    /*else*/ if(type == min_nr_lines_to_draw || type == shift_val
                || type == min_x_range_map || type == max_x_range_map
                || type == min_y_range_map || type == max_y_range_map
                || type == nr_track_points || type == nr_lines_per_screen
                || type == nr_lin_to_ref_str_scan || type == nr_ref_str_scan
                || type == show_colorbar_0 || type == show_colorbar_1)
    {
        if (sscanf(ptr, "%d", &val_i) != 1)
        {
            printf("!!! Parameter value '%s' not valid !!!", ptr);
            return -1;
        }

        switch (type)
        {
        case min_nr_lines_to_draw:
            if (checkIntPar("min_nr_lines_to_draw", val_i, MIN_MIN_NR_LINES_TO_DRAW, MAX_MIN_NR_LINES_TO_DRAW) == 0)
                viewer_config->min_nr_lines_to_draw = val_i;
            else
                return -1;
            break;
        case shift_val:
            if (checkIntPar("shift_val", val_i, MIN_MIN_NR_LINES_TO_DRAW, viewer_config->min_nr_lines_to_draw) == 0)
                viewer_config->shift_val = val_i;
            else
                return -1;
            break;
        case min_x_range_map:
            if (checkIntPar("min_x_range_map", val_i, MIN_MIN_X_RANGE_MAP, MAX_MIN_X_RANGE_MAP) == 0)
                viewer_config->min_x_range_map = val_i;
            else
                return -1;
            break;
        case max_x_range_map:
            if (checkIntPar("max_x_range_map", val_i, MIN_MAX_X_RANGE_MAP, MAX_MAX_X_RANGE_MAP) == 0)
                viewer_config->max_x_range_map = val_i;
            else
                return -1;
            break;
        case min_y_range_map:
            if (checkIntPar("min_y_range_map", val_i, MIN_MIN_Y_RANGE_MAP, MAX_MIN_Y_RANGE_MAP) == 0)
                viewer_config->min_y_range_map = val_i;
            else
                return -1;
            break;
        case max_y_range_map:
            if (checkIntPar("max_y_range_map", val_i, MIN_MAX_Y_RANGE_MAP, MAX_MAX_Y_RANGE_MAP) == 0)
                viewer_config->max_y_range_map = val_i;
            else
                return -1;
            break;
        case nr_track_points:
            if (checkIntPar("nr_track_points", val_i, MIN_NR_TRACK_POINTS, MAX_NR_TRACK_POINTS) == 0)
                viewer_config->nr_track_points = val_i;
            else
                return -1;
            break;
        case nr_lines_per_screen:
            if(checkIntPar("nr_lines_per_screen", val_i, MIN_NR_LINES_PER_SCREEN, MAX_NR_LINES_PER_SCREEN) == 0) {
                viewer_config->nr_lines_per_screen = val_i;
            }
            else
                return -1;
            break;
            // todo:
        case nr_lin_to_ref_str_scan:
            if(checkIntPar("nr_lin_to_ref_str_scan", val_i, MIN_NR_LIN_TO_REF_STR_SCAN, MAX_NR_LIN_TO_REF_STR_SCAN) == 0) {
                viewer_config->nr_lin_to_ref_str_scan = val_i;
            }
            else
                return -1;
            break;
        case nr_ref_str_scan:
            if(checkIntPar("nr_ref_str_scan", val_i, MIN_REF_STR_SCAN, MAX_REF_STR_SCAN) == 0) {
//                if((val_i % 2) != 0) {
//                    val_i++;
//                }
                viewer_config->nr_ref_str_scan = val_i;
            }
            else
                return -1;
            break;
        case show_colorbar_0:
            if(checkIntPar("show_colorbar_0", val_i, 0, 1) == 0) {
                viewer_config->showColorbar[Stream0] = val_i;
            }
            else
                return -1;
            break;
        case show_colorbar_1:
            if(checkIntPar("show_colorbar_1", val_i, 0, 1) == 0) {
                viewer_config->showColorbar[Stream1] = val_i;
            }
            else
                return -1;
            break;
        }
    }
    /* geographic position parameters */
    else if(type == start_pos_map_lat || type == start_pos_map_lon)
    {
        if (sscanf(ptr, "%d'%d'%d", &val_deg, &val_min, &val_sec) != 3)
        {
            printf("!!! Parameter value '%s' not valid !!!\n", ptr);
            return -1;
        }

        switch (type)
        {
            case start_pos_map_lat:
                if (checkGeoCoordPar("start_pos_map_lat", val_deg, val_min, val_sec) == 0) {
                    viewer_config->start_pos_map.lat = GeoConv::exactGeoPos2Deg(val_deg, val_min, val_sec);
                }
                else
                    return -1;
                break;
            case start_pos_map_lon:
                if (checkGeoCoordPar("start_pos_map_lon", val_deg, val_min, val_sec) == 0) {
                    viewer_config->start_pos_map.lon = GeoConv::exactGeoPos2Deg(val_deg, val_min, val_sec);
                }
                else
                    return -1;
                break;
        }
    }

    /* float parameters */
//    if (type == max_x_range_map || type == max_y_range_map)
//    {
//      if (DEBUG_PARSE)
//        printf("Parameter '%s' set to %.2f\n", buf, val_f);
//    }
    /* integer parameters */
    /*else*/ if(type == min_nr_lines_to_draw || type == shift_val ||
                type == min_x_range_map || type == max_x_range_map ||
                type == min_y_range_map || type == max_y_range_map ||
                type == nr_track_points || type == nr_lines_per_screen ||
                type == nr_lin_to_ref_str_scan || type == nr_ref_str_scan ||
                type == show_colorbar_0 || type == show_colorbar_1)
    {
      if (DEBUG_PARSE)
        printf("Parameter '%s' set to %d\n", buf, val_i);
    }
    // geo pos is printed during checking value

    return 0;
}


int SarViewer::ConfigParser::checkFloatPar(const char *par_name, float par_val, float min_val, float max_val) {
    if (par_val >= min_val && par_val <= max_val)
    {
        if (DEBUG_PARSE_CHECK)
            printf("Parameter %s = %.2f is within range <%.2f,%.2f>\n", par_name, par_val, min_val, max_val);
    }
    else
    {
        printf("!!! Parameter %s = %.2f is out of range <%.2f,%.2f> !!!\n", par_name, par_val, min_val, max_val);
        return -1;
    }

    return 0;
}


int SarViewer::ConfigParser::checkIntPar(const char *par_name, int par_val, int min_val, int max_val) {
    if (par_val >= min_val && par_val <= max_val)
    {
        if (DEBUG_PARSE_CHECK)
            printf("Parameter %s = %d is within range <%d,%d>\n", par_name, par_val, min_val, max_val);
    }
    else
    {
        printf("!!! Parameter %s = %d is out of range <%d,%d> !!!\n", par_name, par_val, min_val, max_val);
        return -1;
    }

    return 0;
}

int SarViewer::ConfigParser::checkGeoCoordPar(const char *par_name, int val_deg, int val_min, int val_sec) {
    if (DEBUG_PARSE_CHECK)
        printf("Parameter %s set to %d'%d'%d\n", par_name, val_deg, val_min, val_sec);
    return 0;
}

