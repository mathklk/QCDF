#ifndef FRAME_H
#define FRAME_H

#include "numlist.h"

#include <complex>
#include <stdint.h>
#include <QtMath>
#include <QByteArray>
#include <QByteArray>
#include <QList>
#include <QJsonArray>

typedef union {
    struct {
        int16_t I;
        int16_t Q;
    } iq;
    struct {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
    } bytes;
    uint32_t u32;
} IQ;

class ComplexList: public QList<std::complex<float>> {
public:
    // Complex
    NumList<int16_t> I() const {
        NumList<int16_t> ret;
        ret.reserve(size());
        for (auto const& x : *this) {
            ret.append(x.real());
        }
        return ret;
    }
    NumList<int16_t> Q() const {
        NumList<int16_t> ret;
        ret.reserve(size());
        for (auto const& x : *this) {
            ret.append(x.imag());
        }
        return ret;
    }
    NumList<float> abs() const {
        NumList<float> ret;
        ret.reserve(size());
        for (auto const& x : *this) {
            ret.append(std::abs(x));
        }
        return ret;
    }
    NumList<float> arg() const {
        NumList<float> ret;
        ret.reserve(size());
        for (auto const& x : *this) {
            ret.append(std::arg(x));
        }
        return ret;
    }
    ComplexList shifted(float const angle_rad) const {
        ComplexList ret;
        ret.reserve(size());
        for (auto const& x : *this) {
            ret.append(x * std::polar(1.0f, angle_rad));
        }
        return ret;
    }

    ComplexList sliced(qsizetype pos, qsizetype n) const {
        ComplexList ret;
        ret.reserve(n);
        for (qsizetype i = 0; i < n; ++i) {
            ret.append(this->at(pos + i));
        }
        return ret;
    }

    QJsonArray toJson() const {
        QJsonArray json;
        for (std::complex<float> const& x : *this) {
            QJsonArray pair;
            pair.append(x.real());
            pair.append(x.imag());
            json.append(pair);
        }
        return json;
    }
    static ComplexList fromJson(QJsonArray const& array) {
        ComplexList ret;
        ret.reserve(array.size());
        for (QJsonValue const& value : array) {
            QJsonArray const& pair = value.toArray();
            if (pair.size() != 2 or pair[0].type() != QJsonValue::Type::Double or pair[1].type() != QJsonValue::Type::Double) {
                qWarning() << "Couldn't parse json pair:" << pair;
                continue;
            }
            float const real = pair[0].toDouble();
            float const imag = pair[1].toDouble();
            ret.append(std::complex<float>(real, imag));
        }
        return ret;
    }
};


struct Frame {
    // Header
    uint16_t id = 0;
    uint16_t nBytes;
    uint32_t crc;
    // Content
    QByteArray data;

    // Additional info
    uint32_t calculatedCrc;
    bool crcIsOk = false;

    // Cached complex representation (mutable so it can be updated in a const method)
    mutable ComplexList _complexCache;
    mutable bool _complexCacheValid = false;

    bool isValid() const { return id != 0; }

    ComplexList asComplex() const {
        // Lazy-build and cache the complex list. This keeps the API const
        // while avoiding recomputation on subsequent calls for the same data.
        if (!_complexCacheValid || _complexCache.size() != data.size()/4) {
            _complexCache.clear();
            _complexCache.reserve(data.size()/4);
            for (int i = 0; i < data.size()/4; ++i) {
                IQ iq;
                iq.bytes.b0 = data[i*4    ];
                iq.bytes.b1 = data[i*4 + 1];
                iq.bytes.b2 = data[i*4 + 2];
                iq.bytes.b3 = data[i*4 + 3];
                _complexCache.append(std::complex<float>(iq.iq.I, iq.iq.Q));
            }
            _complexCacheValid = true;
        }
        return _complexCache;
    }
    operator ComplexList() const {
        return this->asComplex();
    }
};

#endif // FRAME_H
