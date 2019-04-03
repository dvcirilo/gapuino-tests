#include "cmsis.h"
#include "gap_common.h"
#include "mbed_wait_api.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>

int main()
{
//#pragma omp parallel num_threads(NUM_THREADS)
//    {
        int a = 10;
        printf("Hello from thread %d\n - %d", 0, a);
    
//    }
    exit(0);
}
