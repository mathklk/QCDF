#include "spectrum.h"

float Spectrum::atAngle(int const& alpha) const {
    for (auto const& [beta, amplitude] : *this) {
        if (alpha == beta) {
            return amplitude;
        }
    }
    return NAN;
}