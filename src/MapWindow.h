#ifndef MAPWINDOW_H
#define MAPWINDOW_H

#include <qmainwindow.h>
#include "MapViewer.h"

class QGraphicsScene;
class QGraphicsView;
class QSignalMapper;

namespace SarViewer {
    class MapWindow : public QMainWindow {
        Q_OBJECT
    public :
        MapWindow(QMainWindow *parent = 0);
        ~MapWindow();
        const MapViewer *getMapViewer() const { return mapViewer_; }

    protected :
        void resizeEvent(QResizeEvent *event);
        void closeEvent(QCloseEvent *event);

    private slots :
        void close();

    signals :
        void mapWindowShowEnable();

    private :
        QMenu *fileMenu_;
        QMenu *optionsMenu_;
        QMenu *mapDetailsMenu_;
        QMenu *viewMenu_;
        QMenu *trackMenu_;
        QMenu *legendMenu_;
        QAction *closeAction_;
        QAction *exitAction_;
        QAction *trackRestartAction_;
        QAction *legendShowAction_;
        QAction *keepAspeRatAction_;
//        QGraphicsScene *scene_;
//        QGraphicsView *view_;

        MapViewer *mapViewer_;

        QSignalMapper *mapDetailSigMap_;
    };
} // namespace SarViewer
#endif // MAPWINDOW_H
