#ifndef __LIBUNRANDOM_H__
#define __LIBUNRANDOM_H__

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t* _unrandom_state(void)
{
    static uint16_t state = 0;
    return &state;
}

static inline uint16_t* _unrandom_state2(void)
{
    static uint16_t state = 0;
    return &state;
}

static inline void unrandom_init(int argc, char **argv)
{
    if (*_unrandom_state() != 0 || *_unrandom_state2() != 0)
        return;
    *_unrandom_state() = argc;
    for(int i = 0; i < argc; i++)
    {
        size_t len = strlen(argv[i]);
        *_unrandom_state2() += len;
        for(size_t j = 0; j < len; j++)
            *_unrandom_state2() += argv[i][j];
    }
}

static inline void unrandom_scroll()
{
    *_unrandom_state() += *_unrandom_state2();
    *_unrandom_state2() += sizeof(*_unrandom_state());
    *_unrandom_state2() = *_unrandom_state() >> 2;
    *_unrandom_state() = *_unrandom_state2() << 4;
}

static inline int unrandom_int()
{
    unrandom_scroll();
    return (int32_t)((uint32_t)*_unrandom_state() << 16 | *_unrandom_state2());
}

static inline float unrandom_float()
{
    return (unrandom_int() % 0x40000000) / (float) 0x40000000;
}

extern int __unrandom_main__(int argc, char **argv);

#ifdef __cplusplus
}
#endif

int main(int argc, char **argv)
{
    unrandom_init(argc, argv);
    return __unrandom_main__(argc, argv);
}

#define main __unrandom_main__

#endif // __LIBUNRANDOM_H__