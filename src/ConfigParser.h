/// @file ConfigParser.h
/// @brief Class providing reading configuration file
/// @author Krzysztof Borowiec

#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#define MAX_NR_PARS 100  /**< maximum number of parameters */
#define MAX_PAR_LEN 30   /**< maximum length of the parameter name [characters] */
#define LINE_LEN 100     /**< maximum length of the line */
#define FILE_LEN 10000   /**< maximum length of the file [characters] */
#define COMMENT_CHAR '#' /**< character denoting single-line comment */

#include <qglobal.h>
#include <qobject.h>
#include "Config.h"


namespace SarViewer {
    class ConfigParser : public QObject {
        Q_OBJECT
    public :
        ConfigParser(QObject *parent = 0);
        ~ConfigParser();
        int parseConfigFile(const QString &file_name, ViewerConfig *viewer_config);
        void printf_pars(ViewerConfig *v_c);

    private :
        int readConfigFile(const QString &file_name, char *file_buf);
        void removeComments(char *buf);
        int getNextLine(char *file_buf, char *line);
        int parseLine(char *line, ViewerConfig *viewer_config, int line_nr);
        int checkFloatPar(const char *par_name, float par_val, float min_val, float max_val); // TODO template
        int checkIntPar(const char *par_name, int par_val, int min_val, int max_val);
        int checkGeoCoordPar(const char *par_name, int val_deg, int val_min, int val_sec);
    };
} // namespace SarViewer

#endif // CONFIGPARSER_H
