#ifndef EVALUATION_H
#define EVALUATION_H

#include <QString>

#include "music.h"
#include "numlist.h"

namespace evaluation {

struct MeanStdRmse {
    double mean;
    double std;
    double rmse;
};

struct Evaluation {
    int N;
    int n;

    struct {
        NumList<double> peaks;
        MeanStdRmse msr;
        double minY;
        double maxY;
    } musicSeparate;

    struct {
        Spectrum spectrum;
        int peak;
        double minY;
        double maxY;
    } musicSum;

    struct {
        MeanStdRmse msr01;
        MeanStdRmse msr12;
        MeanStdRmse msr02;
        MeanStdRmse msr;
    } pdoa;

public:
    QStringList toColumn(void) const;
};

}; // namespace evaluation

#endif // EVALUATION_H
