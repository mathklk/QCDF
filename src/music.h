#ifndef MUSIC_H
#define MUSIC_H

#include <QVector>
#include <QPair>

#include "spectrum.h"
#include "antenna_array_type.h"
#include "frame.h"


/**
 * Wrapper around the gr_doa Music implementations
 *
 * @param array: Array configuration (ULA or UCA)
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
    int& peak,
    double& minY,
    double& maxY
);

#endif // MUSIC_H
