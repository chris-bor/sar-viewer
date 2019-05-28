#ifndef MAPTEXTITEM_H
#define MAPTEXTITEM_H

#include <QtGui>
#include "qcustomplot.h"

const int TEXT_OFFSET_X  = 105;   // offset in pixels by which the text will be shifted
const int TEXT_OFFSET_Y  = 105;   // offset in pixels by which the text will be shifted

namespace SarViewer {

    class MapTextItem : public QCPCurve {
        Q_OBJECT

    public :
        MapTextItem(QCPAxis *keyAxis, QCPAxis *valueAxis);
        virtual ~MapTextItem();

        // Getters
        const QVector<QString> *name() const { return mName; }

        // Setters

        // Non-property methods
        void addName(const QString name_str);
        void clearName();

    protected :
        QVector<QString> *mName;
        virtual void draw(QCPPainter *painter);
        void getDataAndName(QVector<QPointF> *data, QVector<QString> *names) const;
    };
} // namespace SarViewer

#endif // MAPTEXTITEM_H
