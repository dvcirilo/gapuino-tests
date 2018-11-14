// PRINTF
#include "gap_common.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include <time.h>
#include "hal/us_ticker_api.h"

#define CORE_NUMBER      8
#define BILLION 1000000000L

void random_gen(void *arg) {
    int i;
    unsigned int seed=10;
    if(__core_ID() == 4)
        seed = 12;
    for(i=0;i<1000000;i++)
        rand_r(&seed);
    printf("%d\t%d\n", __core_ID(), rand_r(&seed));
}

void Master_Entry(void *arg) {
    CLUSTER_CoresFork(random_gen, arg);
}

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}

int main()
{
    int set_voltage_return;
    /*struct timespec start, end;*/

    printf("Default FC Frequency      = %d kHz\n", FLL_GetFrequency(uFLL_SOC)/1000);
    printf("Voltage: %lu\n", current_voltage());

    /* Set voltage */
    set_voltage_return = PMU_SetVoltage(1100,0);
    if (set_voltage_return) {
        printf("Failed to change voltage! Code: %d\n",set_voltage_return);
    }
    printf("Voltage: %lu - FCMaxFreq: %d kHz - FCMaxFreq: %d kHz\n",
            current_voltage(),FLL_SoCMaxFreqAtV(current_voltage())/1000,
            FLL_ClusterMaxFreqAtV(current_voltage())/1000);

    /* Set frequency */
    if (FLL_SetFrequency(uFLL_SOC, 100000000 , 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }
    printf("FC Freq: %d kHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/1000,current_voltage());
    random_gen(NULL);

    /*clock_gettime(CLOCK_REALTIME, &start);*/

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    /* Set frequency */
    if (FLL_SetFrequency(uFLL_CLUSTER, 2850000, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }
    printf("Cluster Freq: %d kHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_CLUSTER)/1000,current_voltage());

    /* FC send a task to Cluster */
    CLUSTER_SendTask(0, Master_Entry, 0, 0);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    /*clock_gettime(CLOCK_REALTIME, &end);*/
    /*printf("elapsed: %llu us\n", BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec);*/


    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    exit(error);
}
