#ifndef METATYPES_H
#define METATYPES_H

#include "frame.h"

static void registerMetaTypes() {
    qRegisterMetaType<Frame>("Frame");
}

#endif // METATYPES_H
