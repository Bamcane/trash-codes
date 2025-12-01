#ifndef __LIBROUND_H__
#define __LIBROUND_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int64_t libround_to_int(int64_t number) { return number; }
double libround_to_double_but_without_any_float_point(double number) { return (double)(int64_t)number; }

#ifdef __cplusplus
}
#endif

#endif // __LIBROUND_H__