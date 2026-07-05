#ifndef IVECTOR_H
#define IVECTOR_H

#include <QVector>
#include <functional>
#include "numlist.h"

template <typename T>
class IVector : public QVector<T>
{
public:
    IVector() : QVector<T>() {}
    IVector(QVector<T> const& vec) : QVector<T>(vec) {}

    IVector<T> filter(std::function<bool(T const&)> const& predicate) const {
        IVector<T> result;
        for (auto const& item : *this) {
            if (predicate(item)) {
                result.append(item);
            }
        }
        return result;
    }

    template <typename RT>
    IVector<RT> map(std::function<RT(T const&)> const& func) const {
        IVector<RT> result;
        for (auto const& item : *this) {
            result.append(func(item));
        }
        return result;
    }

    operator QVector<T>() const {
        return QVector<T>(*this);
    }
    operator NumList<T>() const {
        NumList<T> ret;
        ret.reserve(this->size());
        for (T const& x : *this) {
            ret.append(x);
        }
        return ret;
    }
};

#endif // IVECTOR_H
