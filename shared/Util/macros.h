#ifndef MACROS_H
#define MACROS_H

#define RECONSTRUCT_UINT16(_data)\
    ((uint16_t)(*((_data) + 0)) << 8)\
    | (*((_data) + 1))

#define RECONSTRUCT_UINT32(_data) \
    ((uint32_t)(*((_data) + 0)) << 24) \
    | ((uint32_t)(*((_data) + 1)) << 16) \
    | ((uint32_t)(*((_data) + 2)) << 8) \
    | (*((_data) + 3))

#endif // MACROS_H