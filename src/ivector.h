#ifndef IVECTOR_H
#define IVECTOR_H

#include <QVector>
#include <functional>

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
};

#endif // IVECTOR_H
