#ifndef DEBUGGING_H
#define DEBUGGING_H

#define DEBUG_VIEWER                    0
#define DEBUG_QPAINTEVENT               0
#define DEBUG_REPLY                     0
#define DEBUG_CONST                     0 // debug constructors
#define DEBUG_DESTR                     0 // debug destructors
#define DEBUG_NET                       0 // debug network connection
#define DEBUG_LOC_SERV_NAME             0 // debug LocalSocketServer class
#define DEBUG_LOC_SOC_REC_THR_RUN       0 // debug the run() function at LocalSocketReceiverThread class
#define DEBUG_LOC_SOC_REC_THR           0 // debug LocalSocketReceiverThread
#define DEBUG_SAR_IMG_DATA_BUFFER       0
#define DEBUG_SAR_SCENE                 1
#define DEBUG_SAR_VIEW                  0
#define DEBUG_PARSE                     0 // debug parsing the configuration file
#define DEBUG_PARSE_CHECK               0 // debug parsing the configuration file functions which checks scope of parameters
#define DEBUG_FILE                      0 // debug reading file
#define DEBUG_MAP_VIEWER_READ           0 // debug reading data at member function of the MapViewer class
#define DEBUG_MAP_VIEWER_PLOT           0 // debug the plotting function of the MapViewer class
#define DEBUG_SIM_MOD_DRAW              0 // debug the simple mode drawer
#define DEBUG_MAIN_WINDOW_UDP_FINISHED  0 // debug function checking deleting udp receiver threads
#define DEBUG_MAIN_WINDOW_DRAW_FINISHED 0
#define DEBUG_MAIN_WINDOW_PLANE_FINISHED 0
//#define QT_NO_DEBUG // on/off debugging fg. assertions

#define PR_FULL( msg ) qDebug() << "File :" << __FILE__ << "\tLine nr :" << __LINE__ << "\tmsg :" << #msg << "=" << msg
#define PR_ERROR( msg ) qDebug() << "ERROR at file :" << __FILE__ << "\tline nr :" << __LINE__ << "\tmsg :" << msg
#define PR_LINE( msg ) qDebug() << "File :" << __FILE__ << "\tLine nr :" << __LINE__ << "\tmsg :" << msg
#define PR_DE( msg ) qDebug() << msg
#define PR( msg ) qDebug() << #msg << "=" << msg
#define PR_MSG( msg ) qDebug() << msg
#define PR_LINE_MSG(msg) qDebug() << "[" <<__LINE__ << "]" <<  __FUNCTION__ << "\t" << msg
#define PR_LINE_VAL(val) qDebug() << "[" << __LINE__ << "]" << __FUNCTION__ << "\t" << #val << "=" << val
#define PR_LINE_MSG_VAL(msg, val) qDebug() << "[" << __LINE__ << "]" << __FILE__ << "\t" << msg << ":" << val

#endif // DEBUGGING_H
