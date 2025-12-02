#include "libround.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    int round = argc * 123;
    double number = round * 221312451.21431321523702;
    printf("before: %d, after: %ld\n", round, libround_to_int(round));
    printf("before: %lf, after: %lf\n", number, libround_to_double_but_without_any_float_point(number));
    return 0;
}