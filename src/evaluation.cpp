#include "evaluation.h"

#include <QStringList>

QStringList evaluation::Evaluation::toColumn() const {
    return {
        // General
        QString::number(N),
        QString::number(n),
        // MUSIC Separate
        QString::number(musicSeparate.msr.mean, 'f', 2),
        QString::number(musicSeparate.msr.std, 'f', 2),
        QString::number(musicSeparate.msr.rmse, 'f', 2),
        // MUSIC Sum
        QString::number(musicSum.peak),
        QString::number(musicSum.quality, 'f', 2),
        // PDOA
        QString::number(pdoa.msr.mean, 'f', 2),
        QString::number(pdoa.msr.std, 'f', 2),
        QString::number(pdoa.msr.rmse, 'f', 2),
        QString::number(pdoa.msr02.mean, 'f', 2),
        QString::number(pdoa.msr02.std, 'f', 2),
        QString::number(pdoa.msr02.rmse, 'f', 2)
    };
}
