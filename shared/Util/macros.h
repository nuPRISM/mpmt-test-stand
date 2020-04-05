#ifndef MACROS_H
#define MACROS_H

/******************************************************************************/
/* BYTE ORDER MACROS                                                          */
/******************************************************************************/

#define NTOHS(_ptr)                                \
    (                                              \
    ((uint16_t)(*((uint8_t *)(_ptr) + 0)) << 8)  | \
    ((uint16_t)(*((uint8_t *)(_ptr) + 1)) << 0)    \
    )

#define NTOHL(_ptr)                                \
    (                                              \
    ((uint32_t)(*((uint8_t *)(_ptr) + 0)) << 24) | \
    ((uint32_t)(*((uint8_t *)(_ptr) + 1)) << 16) | \
    ((uint32_t)(*((uint8_t *)(_ptr) + 2)) << 8)  | \
    ((uint32_t)(*((uint8_t *)(_ptr) + 3)) << 0)    \
    )

#define GET_BYTE(_x, _n) ((uint8_t)(((_x) >> ((_n)*8)) & 0xFF))

#define HTONS(_ptr, _val)                               \
    do {                                                \
        (*((uint8_t *)(_ptr) + 0)) = GET_BYTE(_val, 1); \
        (*((uint8_t *)(_ptr) + 1)) = GET_BYTE(_val, 0); \
    } while (0)

#define HTONL(_ptr, _val)                               \
    do {                                                \
        (*((uint8_t *)(_ptr) + 0)) = GET_BYTE(_val, 3); \
        (*((uint8_t *)(_ptr) + 1)) = GET_BYTE(_val, 2); \
        (*((uint8_t *)(_ptr) + 2)) = GET_BYTE(_val, 1); \
        (*((uint8_t *)(_ptr) + 3)) = GET_BYTE(_val, 0); \
    } while (0)

#endif // MACROS_H