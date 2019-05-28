#include <QApplication>
#include <iostream>
#include <qapplication.h>
#include <stdio.h>
#include <stdlib.h>
#include "MainWindow.h"
#include "Config.h"
#include "LocalSocketReceiverThread.h"
#include "sar_types.h"
#include "ConfigParser.h"

extern ViewerConfig viewer_config;

void myMessageOutput(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s\n", msg);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", msg);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", msg);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", msg);
        abort();
    }
}

int main(int argv, char **args)
{
//#ifdef Q_WS_WIN
//    qDebug() << "Operation System Recognition Successful : Windows";
//#elif defined Q_WS_X11
//    qDebug() << "Operation System Recognition Successfull : Linux";
//#else
//    qDebug() << "Operation System Recognition Unsiccessful";
//    exit(-1);
//#endif

    QTextCodec::setCodecForTr(QTextCodec::codecForName ("UTF-8"));

//    qInstallMsgHandler(myMessageOutput);

    QApplication app(argv, args);
    app.setOrganizationName("RDSP");
    app.setApplicationName("SAR viewer");

    // Register new types
//    qRegisterMetaType<SarViewer::LocalSocketReceiverThread>(); // TO DO KONSTRUKTOR KOPIUJACY
    qRegisterMetaType<BYTE>("BYTE");
    qRegisterMetaType<INT8>("INT8");
    qRegisterMetaType<UINT8>("UINT8");
    qRegisterMetaType<INT16>("INT16");
    qRegisterMetaType<UINT16>("UINT16");
    qRegisterMetaType<INT32>("INT32");
    qRegisterMetaType<UINT32>("UINT32");
    qRegisterMetaType<FLOAT32>("FLOAT32");
    qRegisterMetaType<FLOAT>("FLOAT");
    qRegisterMetaType<FLOAT64>("FLOAT64");
    qRegisterMetaType<DOUBLE>("DOUBLE");
    qRegisterMetaType<sar_image_line_header>("sar_image_line_header");

    // Read viewer configuration file
    SarViewer::ConfigParser *configParser = new SarViewer::ConfigParser();
    configParser->parseConfigFile(DEF_CONFIG_FILE_NAME, &viewer_config);
    configParser->printf_pars(&viewer_config);
    delete configParser;

    SarViewer::MainWindow *mainWindow = new SarViewer::MainWindow;

    //mainWindow.QGLWidget::show();
    mainWindow->show();

    return app.exec();
}

// todo:
// rozdzielczosc ekranu wczytywana z pliku b¹dŸ jeœli siê da to jak¹œ funkcj¹ czytana
// min z rozdzielczoœci ekranu
// nr_track_points okreœla tylko trajektoriê a strip scan siê nie zmniejsza
