#pragma once
#include "common/types.h"
#include "coreinit_enum.h"
#include "common/be_val.h"

namespace coreinit
{

void *
OSBlockMove(void *dst,
            const void *src,
            ppcsize_t size,
            BOOL flush);

void *
OSBlockSet(void *dst,
           uint8_t val,
           ppcsize_t size);

BOOL
OSGetForegroundBucket(be_val<uint32_t> *addr,
                      be_val<uint32_t> *size);

BOOL
OSGetForegroundBucketFreeArea(be_val<uint32_t> *addr,
                              be_val<uint32_t> *size);

int
OSGetMemBound(OSMemoryType type,
              be_val<uint32_t> *addr,
              be_val<uint32_t> *size);

} // namespace coreinit
