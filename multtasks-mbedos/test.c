// PRINTF
#include "gap_common.h"
#include "gap_cluster.h"
#include <stdlib.h>

#define CORE_NUMBER      8
#define PERF_Msk PCER_CYCLE_Msk
#define PERF_Pos PCER_CYCLE_Pos

performance_t perf[9];

void hello(void *arg) {
    PERFORMANCE_Start(&perf[__core_ID()], PERF_Msk);
    for (int i = 0; i < (int) 10*__core_ID(); i++) {
        rand();
    }
    PERFORMANCE_Stop(&perf[__core_ID()]);
    printf("perf %d - %p, fc? %d, core %d\n" , (int)PERFORMANCE_Get(&perf[__core_ID()], PERF_Pos), &perf[__core_ID()], __is_FC(), __core_ID());
}

void master_entry(void *arg) {
    CLUSTER_CoresFork(hello, arg);
}

int main()
{
    printf("Fabric controller code execution for mbed_os Cluster Power On test\n");

    PERFORMANCE_Start(&perf[8], PERF_Msk);
    /*asm volatile("nop");*/
    PERFORMANCE_Stop(&perf[8]);

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    /* FC send a task to Cluster */
    CLUSTER_SendTask(0, master_entry, 0, 0);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);
    printf("perf %d - %p, fc? %d, core %d\n" , (int)PERFORMANCE_Get(&perf[8], PERF_Pos), &perf[8], __is_FC(), __core_ID());

    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    #ifdef JENKINS_TEST_FLAG
    exit(error);
    #else
    return error;
    #endif
}
