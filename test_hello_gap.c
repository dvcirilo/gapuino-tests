#include "cmsis.h"
#include "gap_common.h"
#include "mbed_wait_api.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>

#define NUM_THREADS   (8)

int omp_get_thread_num(){
    return __core_ID();
}

void parallel_function(void * args)
{
    int a = 10*omp_get_thread_num();
    printf("Hello from thread %d\n - %d", omp_get_thread_num(), a);
}

void Master_Entry(void * args)
{
    CLUSTER_CoresFork(parallel_function, (void *) args);
}

int main()
{
    /* Cluster Start - Power on */
    CLUSTER_Start(0, NUM_THREADS);

    CLUSTER_SendTask(0, Master_Entry, NULL, 0);

    CLUSTER_Wait(0);

    exit(0);
}
