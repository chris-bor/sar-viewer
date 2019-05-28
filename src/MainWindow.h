/// @file MainWindow.h
/// @brief class main window responsible for managing main window
/// @author Krzysztof Borowiec

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtGui>
#include <QObject>
//#include <QGLWidget> // add QT  += opengl at .pro file
#include "View.h"
#include "SimpleMode.h"
//#include "NetworkReceiver.h"
//#include "LocalSocketReceiver.h"
//#include "LocalSocketReceiverThread.h"
#include "SimpleModeDrawer.h"
#include "UdpReceiver.h"
#include "Config.h"
#include "DockWidget.h"
#include "Led.h"
#include "Scene.h"

namespace SarViewer
{
    class MapWindow;

    class MainWindow : public QMainWindow //, public QGLWidget
    {
        Q_OBJECT // processed by MOC; for signals/slots

    public :
        MainWindow(QMainWindow *parent = 0);
        ~MainWindow();
        inline View *getView(int nr_stream) const // returns pointer to type object of View
        {
            return (this->view_[nr_stream]);
        }
        inline Scene *getScene(int nr_stream) const { // Returns pointer to type object of QGraphicsScene
            return (this->scene_[nr_stream]);
        }
        QString getSARDataFileName();
        friend class SimpleModeDrawer;
        DockWidget *getPtrStreamDockWidget(int nr_stream) const {
                return(streamDockWidget_[nr_stream]);
        }

    signals :
        void resized(QSize size);

    public slots:
        void setPlatParams(double lat, double lon, FLOAT alt, FLOAT head, FLOAT pitch, FLOAT roll,
                           FLOAT veloc, UINT32 anal_sett, UINT32 nr_range_cells);
        void changeAction(bool val, int kind);
        void mapWindowShowEnable();
        void onUdpRecvThrFinished0();
        void onUdpRecvThrFinished1();
        void onSimpModDrawThrFinished0();
        void onSimpModDrawThrFinished1();
        void onPlaneThrFinished();
        void onQuitApp();

    protected :
        void paintEvent(QPaintEvent *event);
        void resizeEvent(QResizeEvent *event);
        void closeEvent(QCloseEvent *event);

    private slots :
        void about();
        void setTime();
        void mapWindowShow();
        void showMapWindow();

    private :
        void createActions();
        void createMenus();
        void createStatusBar();
        void createView();
        void createPlatParamDockWidget();
        void createDockWidgets();
        void removeContextMenuFromAllWidgets();

        const QString APP_NAME; // Keeps name of MainWindow
        QSize windowSize_; // Keeps size of MainWindow

        QMenu *fileMenu_;
        QMenu *viewMenu_;
        QMenu *optionsMenu_;
        QMenu *paletteMenu_;
        QMenu *helpMenu_;
        QMenu *showColorbarMenu_;

        QAction *aboutAction_;
        QAction *exitAction_;
        /*QAction *replayAction_;*/
        QAction *simpleModeFromNetworkAction_;
        /*QAction *simpleModeFromLocSockThrAction_;*/
        QAction *paletteAction_;
        QSignalMapper *paletteSigMap_;
        QAction *viewAction_;
        QAction *mapWindowShowAction_;
        QAction *showColorbarAction_[NrStreams];

        View *view_[NrStreams]; // Widget to visualize the scene, it shows what is at scene.
        Scene *scene_[NrStreams]; // Graphics View provides a surface for managing and interacting. Has no visual appearance of its own

        QSize widgetSize_;// Size of widget
        QSize scenePixmapSize_; // Size of pixmap on the scene
        SimpleMode *simpleMode_; // Object to manage simple mode of viewing
        QString fileName_; // String with file name choosed by user

        DockWidget *streamDockWidget_[NrStreams];
        QDockWidget *paramDockWidget_;

        QTextEdit *currTimeTextEdit_;
        QTextEdit *platTextEdit_;
        QTextEdit *radarTextEdit_;

        Led *flagsLed_[NR_FLAG_BITS];
        Led *gpsAvailLed_;
        Led *gpsValidLed_;

/*        NetworkReceiver *networkReceiver_;
        LocalSocketReceiverThread locSockRecvThr_;*/
        UdpReceiver *udpRecvStream0_;
        UdpReceiver *udpRecvStream1_;
        QThread *udpRecvThr_[NrStreams];
        QThread *simpleModeDrawerThr_[NrStreams];
        QThread *planeThr_;

        QLabel *imgXPosLabel_;
        QLabel *imgYPosLabel_;

        QTimer *currTimer_;
        QTime *currTime_;
        QLocale englishLocale_;

        MapWindow *mapWindow_;

        bool closeAllWindows_;
        bool onQuit_;
    };

} // namespace

#endif // MAIN_WINDOW_H
