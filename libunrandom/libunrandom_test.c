#include "libunrandom.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    float proba;
    do
    {
        proba = unrandom_float();
        printf("got unrandom: %d %f\n", unrandom_int(), proba);
    } while (proba < 0.8f);
    return 0;
}