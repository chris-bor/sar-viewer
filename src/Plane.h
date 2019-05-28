#ifndef PLANE_H
#define PLANE_H

#include "qcustomplot.h"
#include "sar_image.h"

#define TRACK_COLOR                 Qt::magenta
#define TRACK_WIDTH                 2
#define PLANE_COLOR                 Qt::green
#define PLANE_RECT_SIZE             30
#define STRIP_SCAN_CURVE_COLOR      PLANE_COLOR
#define STRIP_SCAN_CURVE_WIDTH      1

typedef struct TwoCartCoordType_ TwoCartCoordType;
typedef struct SpecialPoint_ SpecialPoint;

namespace SarViewer {
    class StripScan;
    class StripScanImage;

    class Plane : public QCPAbstractItem {
        Q_OBJECT
    public :
        enum TracerStyle { tsNone        ///< The tracer is not visible
                           ,tsPlus       ///< A plus shaped crosshair with limited size
                           ,tsCrosshair  ///< A plus shaped crosshair which spans the complete axis rect
                           ,tsCircle     ///< A circle
                           ,tsSquare     ///< A square
                           ,tsPlane      ///< A plane
                         };
        Q_ENUMS(TracerStyle)

        Plane(SpecialPoint *start_point, QCustomPlot *parent_plot = 0);
        virtual ~Plane();

        // getters:
        QPen pen() const { return mPen; }
        QPen selectedPen() const { return mSelectedPen; }
        QBrush brush() const { return mBrush; }
        QBrush selectedBrush() const { return mSelectedBrush; }
        double size() const { return mSize; }
        TracerStyle style() const { return mStyle; }
        QCPCurve *curve() const { return mCurve; }
        double curveKey() const { return mCurveKey; }
        bool interpolating() const { return mInterpolating; }
        QCPCurve *getTrackCurve() const { return trackCurve_; }
        double heading() const { return mHeading; }
        StripScanImage *stripScanImage() const { return stripScanImage_; }

        // setters;
        void setPen(const QPen &pen);
        void setSelectedPen(const QPen &pen);
        void setBrush(const QBrush &brush);
        void setSelectedBrush(const QBrush &brush);
        void setSize(double size);
        void setStyle(TracerStyle style);
        void setCurve(QCPCurve *curve);
        void setCurveKey(double key);
        void setInterpolating(bool enabled);
        void setHeading(double heading);

        // non-property methods:
        virtual double selectTest(const QPointF &pos) const;
        void updatePosition();

        QCPItemPosition * const position;

    protected :
        QPen mPen, mSelectedPen;
        QBrush mBrush, mSelectedBrush;
        double mSize;
        TracerStyle mStyle;
        QCPCurve *mCurve;
        double mCurveKey;
        bool mInterpolating;
        double mHeading;

        SpecialPoint *startPoint_;

        virtual void draw(QCPPainter *painter);

        // helper functions:
        QPen mainPen() const;
        QBrush mainBrush() const;

    public slots :
        void addTrackPoint(sar_image_line_header header, int most_actual_line);
        void trackRestart();

    signals :
        void replot();

    private :
        void createPath(); // Create path_
        void addStripScanPoint(sar_image_line_header &header, TwoCartCoordType &p_c);

        QCustomPlot *parent_;
        QVector<sar_image_line_header> headerVect_;
        QCPCurve *trackCurve_;
        StripScan *stripScanCurve_;
        StripScanImage *stripScanImage_;
        QPainterPath path_; // Painter path of the plane
    };
} // namespace SarViewer
#endif // PLANE_H
