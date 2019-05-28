#include <QtGui>
#include "View.h"
#include "Config.h"
#include "Debugging.h"
#include "Scene.h"

namespace SarViewer {

    View::View(Scene *scene) : QGraphicsView(scene) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF View(overloaded)\n";
#endif
    }

    View::View(QGraphicsScene *scene) : QGraphicsView(scene) {
#if DEBUG_CONST
        qDebug() << "CONSTRUCTOR OF View(overloaded)\n";
#endif
    }


    View::~View() {
#if DEBUG_DESTR
        qDebug() << "DESTRUCTOR OF View\n";
#endif
    }

    void View::resizeEvent(QResizeEvent *event) {
#if DEBUG_SAR_VIEW
//        qDebug() << __FUNCTION__ << "()";
#endif
        QGraphicsView::resizeEvent(event);
        fitInView(sceneRect(), Qt::IgnoreAspectRatio);
    }
} // SarViewer
