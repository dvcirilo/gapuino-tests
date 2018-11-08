// PRINTF
#include "gap_common.h"
#include <pthread.h>

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>

#define CORE_NUMBER      8

void Hello(void *arg) {
    int i;
    for(i=0;i<5;i++)
        printf("Hello World #%d from cluster core %d! - rand: %d\n", i, __core_ID(), rand());
}

void Master_Entry(void *arg) {
    CLUSTER_CoresFork(Hello, arg);
}

int main()
{
    srand(10);
    printf("Fabric controller code execution for mbed_os Cluster Power On test\n");
    printf("Test %d %b\n",__core_ID(),4);

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    /* FC send a task to Cluster */
    CLUSTER_SendTask(0, Master_Entry, 0, 0);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    exit(error);
}
