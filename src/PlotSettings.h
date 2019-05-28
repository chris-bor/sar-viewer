#ifndef PLOTSETTINGS_H
#define PLOTSETTINGS_H

#include <qglobal.h>

namespace SarViewer
{
    class PlotSettings {
    public:
        PlotSettings();
        void scroll(int dx, int dy);
        void adjust();
        double spanX() const { return maxX_ - minX_; }
        double spanY() const { return maxY_ - minY_; }
        int minXRound() const { return qRound(minX_); }
        int maxXRound() const { return qRound(maxX_); }
        int minYRound() const { return qRound(minY_); }
        int maxYRound() const { return qRound(maxY_); }
        int spanXRound() const { return maxXRound() - minXRound(); }
        int spanYRound() const { return maxYRound() - minYRound(); }

        int minXRound_;
        int maxXRound_;
        int minYRound_;
        int maxYRound_;
        int spanXRound_;
        int spanYRound_;
        double minX_;
        double maxX_;
        int numXTicks_;
        double minY_;
        double maxY_;
        int numYTicks_;
    private:
        static void adjustAxis(double &min, double &max, int &numTicks);
    };
}
#endif // PLOTSETTINGS_H
