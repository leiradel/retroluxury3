#ifndef DJB2_H__
#define DJB2_H__

#include <inttypes.h>

#define PRIDJB2HASH "0x%08" PRIx32
#define DJB2HASH_C UINT32_C

typedef uint32_t djb2_hash;

djb2_hash djb2(char const* str);

#endif // DJB2_H__
