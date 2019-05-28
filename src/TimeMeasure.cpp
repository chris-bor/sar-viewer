#include "TimeMeasure.h"

void TimeMeasure::timeStart() {
    t1_ = clock();
}

void TimeMeasure::timeStop(const char msg[]) {
    t2_ = clock();
    qDebug( "Time of %s : %.3f s\n", msg, (float)(t2_-t1_)/(float)CLOCKS_PER_SEC);
}


void TimeMeasure::overallTimeStart() {
    tStart_ = clock();
}


void TimeMeasure::overallTimeStop(const char msg[]) {
    tStop_ = clock();
    qDebug( "Overall time of %s: %.3f s\n", msg, (float)(tStop_-tStart_)/(float)CLOCKS_PER_SEC);
}
