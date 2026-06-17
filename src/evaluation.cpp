#include "evaluation.h"

#include <QStringList>

QStringList evaluation::Evaluation::toColumn() const {
    return {
        QString::number(N),
        QString::number(n),
        QString::number(musicSeparate.msr.mean, 'f', 2),
        QString::number(musicSeparate.msr.std, 'f', 2),
        QString::number(musicSeparate.msr.rmse, 'f', 2),
        QString::number(musicSum.peak),
        QString::number(pdoa.msr.mean, 'f', 2),
        QString::number(pdoa.msr.std, 'f', 2),
        QString::number(pdoa.msr.rmse, 'f', 2)
    };
}
