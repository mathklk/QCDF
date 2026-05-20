#ifndef PHYSICS_H
#define PHYSICS_H

#include <QtTypes>

namespace physics {

constexpr quint64 speedOfLightInVacuum_mps = 299'792'458ull;
constexpr double  refractiveIndexOfAir     = 1.0003;
constexpr double  speedOfLightInAir_mps    = speedOfLightInVacuum_mps / refractiveIndexOfAir;

};

#endif // PHYSICS_H
