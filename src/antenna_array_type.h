#ifndef ANTENNA_ARRAY_TYPE_H
#define ANTENNA_ARRAY_TYPE_H

#include <QString>

class AntennaArrayType {
public:
    enum Value: quint8 {
        ULA = 0,
        UCA = 1
    };

    AntennaArrayType() = default;
    constexpr AntennaArrayType(Value v) : value(v) {}
    constexpr AntennaArrayType(int v) : value(static_cast<Value>(v)) {}

    constexpr operator Value() const { return value; }
    operator QString() const {
        switch (value) {
            case ULA: return "ULA";
            case UCA: return "UCA";
            default: return "Unknown";
        }
    }

private:
    Value value;
};


#endif // ANTENNA_ARRAY_TYPE_H
