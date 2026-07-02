#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <QVector>
#include <QPair>

class Spectrum: public QVector<QPair<int,float>> {
public:
    float atAngle(int const&) const;
};

#endif // SPECTRUM_H
