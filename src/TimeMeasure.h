#ifndef TIMEMEASURE_H
#define TIMEMEASURE_H
#include <QString>
#include <time.h>
#include <QDebug>

class TimeMeasure {
public :
    void timeStart();
    void timeStop(const char msg[]);
    void overallTimeStart();
    void overallTimeStop(const char msg[]);

private :
    clock_t t1_;
    clock_t t2_;
    clock_t tStart_;
    clock_t tStop_;
};
#endif // TIMEMEASURE_H

/*
    QTime t;
    t.start();

    // do some things here

    qDebug("things tooks %i ms", t.elapsed());
    */
