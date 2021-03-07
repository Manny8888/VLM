#ifndef _SWAPBYTES_
#define _SWAPBYTES_

#include <byteswap.h>

#define bswap32_block(p, n)                                                                                            \
    {                                                                                                                  \
        size_t nWords = (n + 3) / 4, i;                                                                                \
        uint32_t *wordP = (uint32_t *)p;                                                                               \
        for (i = 0; i < nWords; i++, wordP++) {                                                                        \
            *wordP = bswap_32(*wordP);                                                                                 \
        }                                                                                                              \
    }

#define bswap16_block(p, n)                                                                                            \
    {                                                                                                                  \
        size_t nWords = (n + 1) / 2, i;                                                                                \
        uint16_t *wordP = (uint16_t *)p;                                                                               \
        for (i = 0; i < nWords; i++, wordP++) {                                                                        \
            *wordP = bswap_16(*wordP);                                                                                 \
        }                                                                                                              \
    }

#endif
