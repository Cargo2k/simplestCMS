#include <stdio.h>
#include <config.h>

int
main(int argc, char* argv[])
{
    printf("Hello world\n");
    printf(PACKAGE_STRING);
    printf("\n");
    return 0;
}

