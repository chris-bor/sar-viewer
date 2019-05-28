/// @file Scene.h
/// @brief class provide handling items on scene, get position of mouse,
/// @author Krzysztof Borowiec

#ifndef SCENE_H
#define SCENE_H

#include <qgraphicsscene.h>
#include "PlotSettings.h"

#define MIN_ZOOM_X      10
#define MIN_ZOOM_Y      10

class QRubberBand;
class QGraphicsRectItem;

namespace SarViewer {
    class Scene : public QGraphicsScene {
        Q_OBJECT
    public :
        Scene(qreal x, qreal y, qreal width, qreal height, int nr_stream  = -1, QObject * parent = 0);
        virtual ~Scene();
        void setPlotSettings(const PlotSettings &settings);

    signals :
        void zoomChanged();

    public slots :
        void zoomIn();
        void zoomOut();
        void showColorbar(bool toggled);

    protected :
        void mousePressEvent(QGraphicsSceneMouseEvent *event);        
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    private :
        int nrStream_;

        bool rubberBandIsShown_;
        QRectF rubberBandRect_;
        QGraphicsRectItem *rectItem_;
        QPen rubberBandPen_;
        QBrush rubberBandBrush_;
        bool colorbar_;
        int colorbarWidth_;
    };

} // namespace SarViewer

#endif // SCENE_H
