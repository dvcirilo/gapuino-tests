// PRINTF
#include "gap_common.h"
#include <stdlib.h>

int main()
{
    int i, n = 100, t1 = 0, t2 = 1, nextTerm;

    printf("Fibonacci Series: \n");

    for (i = 1; i <= n; ++i)
    {
        printf("%d, ", t1);
        nextTerm = t1 + t2;
        t1 = t2;
        t2 = nextTerm;
    }
    exit(0);
}
