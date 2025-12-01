#include "libround.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int round = argc * 123;
    double number = round * 221312451.21431321523702;
    printf("before: %d, after: %d\n", round, libround_to_int(round));
    printf("before: %lf, after: %lf\n", number, libround_to_double_but_without_any_float_point(number));
    return 0;
}