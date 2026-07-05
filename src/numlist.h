#ifndef NUMLIST_H
#define NUMLIST_H

#include <QList>
#include <QtMath>

template<typename T>
class NumList: public QList<T> {
public:
    T min(void) const {
        T ret = std::numeric_limits<T>::max();
        for (auto const& x : *this) {
            if (x < ret) {
                ret = x;
            }
        }
        return ret;
    }
    T max(void) const {
        T ret = std::numeric_limits<T>::lowest();
        for (auto const& x : *this) {
            if (x > ret) {
                ret = x;
            }
        }
        return ret;
    }

    // Natural Log
    NumList<float> ln(void) const {
        NumList<float> ret;
        ret.reserve(this->size());
        for (auto const& x : *this) {
            ret.append(qLn<float>(x));
        }
        return ret;
    }

    // Arithmetic Mean
    float mean(void) const {
        float sum = 0;
        for (auto const& x : *this) {
            sum += x;
        }
        return sum / this->size();
    }

    // Standard Deviation
    float stdDev(void) const {
        float const m = mean();

        float variance = 0;
        for (auto const& x : *this) {
            variance += (x - m) * (x - m);
        }
        variance /= this->size();

        return qSqrt(variance);
    }

    QString toQString() const {
        QStringList strList;
        for (auto const& x : *this) {
            strList.append(QString::number(x));
        }
        return "[" + strList.join(", ") + "]";
    }
};

#endif // NUMLIST_H
