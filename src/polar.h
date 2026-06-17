/* Verschiedene Hilfsfunktion rund um Winkel, Polarkoordinaten und komplexe Zahlen
 */
#ifndef POLAR_H
#define POLAR_H

#include "numlist.h"

#include <QList>
#include <QtMath>

#include <complex>

namespace polar {

/* The circularMean and circularStdDev definitons are taken from
 * "Directional Statistics" 1999 by Mardia and Jupp pages 15ff
 * (The same defintions youll find on wikipedia and in many libraries)
 */

template <typename T>
double circularMeanDeg(QList<T> const& angles) {
    if (angles.empty()) {
        return NAN;
    }

    double sum_sin = 0.0;
    double sum_cos = 0.0;

    for (double angle : angles) {
        sum_sin += qSin(qDegreesToRadians(angle));
        sum_cos += qCos(qDegreesToRadians(angle));
    }

    // Angles are spread extremely evenly. No plausible mean angle, we return NAN.
    if (std::abs(sum_sin) < 1e-9 and std::abs(sum_cos) < 1e-9) {
        return NAN;
    }

    // atan2 handles the quadrants correctly based on the signs of sin and cos
    return qRadiansToDegrees(qAtan2(sum_sin, sum_cos));
}

template <typename T>
double circularStdDevDeg(QList<T> const& angles) {
    if (angles.empty()) {
        return NAN;
    }

    double sum_sin = 0.0;
    double sum_cos = 0.0;

    for (double angle : angles) {
        sum_sin += qSin(qDegreesToRadians(angle));
        sum_cos += qCos(qDegreesToRadians(angle));
    }

    double R = std::sqrt(sum_sin * sum_sin + sum_cos * sum_cos) / angles.size();
    R = qMin(1.0, R); // catches rare floating point accumulations >1
    return qRadiansToDegrees(std::sqrt(-2.0 * qLn(R)));
}

// Root Mean Square Error
template <typename T>
double rmse(QList<T> const& samples, T const& truth, QPair<T,T> const*const wrapRange = nullptr) {

    auto wrapError = [&](T error) -> T {
        if (wrapRange == nullptr) {
            return error;
        }
        const T lo    = wrapRange->first;
        const T hi    = wrapRange->second;
        const T range = hi - lo;

        error = std::fmod(error - lo, range);
        if (error < 0) error += range;
        return error + lo;
    };

    NumList<T> squaredErrors;
    for (const auto& sample : samples) {
        T wrappedError = wrapError(sample - truth);
        squaredErrors << wrappedError * wrappedError;
    }
    return std::sqrt(squaredErrors.mean());
}

// Amplitude einer komplexen Zahl logarithmieren
std::complex<float> cLn(std::complex<float> const& z) {
    float const A = std::abs(z);
    if (A == 0.0f) {
        // ln(0) ist nicht definiert; Rueckgabe von 0 als sinnvolle Konvention.
        return {0.0f, 0.0f};
    }
    float const scale = qLn(A) / A;  // Skalierungsfaktor (reell)
    return z * scale;
}

double wrapPi(double x) {
    while (x <= -M_PI) x += 2*M_PI;
    while (x >   M_PI) x -= 2*M_PI;
    return x;
}
double wrap180(double x) {
    while (x <= -180) x += 2*180;
    while (x >   180) x -= 2*180;
    return x;
}

}; // namespace polar

#endif // POLAR_H
