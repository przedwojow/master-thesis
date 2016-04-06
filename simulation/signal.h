#ifndef SIGNAL_H
#define SIGNAL_H

#include <iostream>
#include<list>
#include<vector>
#include <QObject>
#include <QVBoxLayout>
#include<cmath>
#include "constants.h"

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

class Signal : public QObject
{
    Q_OBJECT
public:
    explicit Signal(QObject *parent = 0);
    ~Signal();

    void addTime(double time);
    void clear();
    void showSignals();

    void setSignalPlot(QwtPlotCurve *value);
    void setPlot(QwtPlot *value);

    std::vector<double> getSignalProbes() const;
    QwtPlotCurve *getSignalPlot() const;
    QwtPlot *getPlot() const;

private:
    double signalMin = 0;
    double signalMax = 1500;
    double signalStep = 1;
    std::list<double> times;
    std::vector<double> signalProbes;

    QwtPlot* plot;
    QwtPlotCurve *signalPlot;

    void generateSignal();
    double computeSignalValueInTime(double time);
};

#endif // SIGNAL_H