#include "libexists.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    char chr = getchar();
    exists_if(chr != 'Y') printf("You only live once\n");
    exists_if(wonderful_world) printf("What a depressing world...\n");
    return 0;
}