#ifndef EVALUATION_H
#define EVALUATION_H

#include <QString>

#include "music.h"
#include "numlist.h"

namespace evaluation {

struct MeanMedianStdRmse {
    double mean;
    double median;
    double std;
    double rmse;
};

struct Evaluation {
    int N;
    int n;

    struct {
        QList<Spectrum> spectra; // }
        NumList<double> peaks;   // } Only those with pong
        MeanMedianStdRmse msr;
        double minY;
        double maxY;
    } musicSeparate;

    struct {
        Spectrum spectrum;
        int peak;
        double quality;
        double minY;
        double maxY;
    } musicSum;

    struct {
        NumList<double> alpha01;
        NumList<double> alpha12;
        NumList<double> alpha02;
        MeanMedianStdRmse msr01;
        MeanMedianStdRmse msr12;
        MeanMedianStdRmse msr02;
        MeanMedianStdRmse msr;
    } pdoa;

public:
    QStringList toColumn(void) const;
};

}; // namespace evaluation

#endif // EVALUATION_H
