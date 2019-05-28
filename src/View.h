/// @file View.h
/// @brief class responsilble for viewing images and pixel maps
/// @author Krzysztof Borowiec

#ifndef VIEW_H
#define VIEW_H

#include <qgraphicsview.h>
#include <qgraphicsscene.h>
#include "Scene.h"


namespace SarViewer {

    class View : public QGraphicsView {
    public:
        View(SarViewer::Scene *scene);
        View(QGraphicsScene *scene);
        ~View();

    protected :
        void resizeEvent(QResizeEvent *event);
    };

} // namespace
#endif // VIEW_H
