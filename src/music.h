#ifndef MUSIC_H
#define MUSIC_H

#include <QVector>
#include <QPair>

#include "antenna_array_type.h"
#include "frame.h"

class Spectrum: public QVector<QPair<int,float>> {
public:
    float atAngle(int const&) const;
};

/**
 * @param array: Either "ULA" or "UCA"
 * @param dLambda: Normalized Antenna Spacing (e.g. 0.5 for D = 0.5 wavelengths)
 * @param collection:
 * @param cali: start and end of the calibration period (indices of collection)
 * @param pong: start and end of the signal to be df'd
 * @return
 */
Spectrum music(
    AntennaArrayType const array,
    float const dLambda,
    QVector<ComplexList> const& collection,
    QPair<int, int> cali,
    QPair<int, int> pong,
    int& peak,
    double& minY,
    double& maxY
);

/**
 * @brief Normalizes a spectrum to -1..0
 * Assumes the spectrum only contains negative values
 *
 * @param spectrum
 * @param minY Optional, if already known, the min value
 */
void normalizeSpectrum(Spectrum& spectrum, float minY = INFINITY);

#endif // MUSIC_H
