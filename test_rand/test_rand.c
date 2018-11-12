// PRINTF
#include "gap_common.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
/*#include <time.h>*/
#include "hal/us_ticker_api.h"

#define CORE_NUMBER      8

void random_gen(void *arg) {
    int i;
    unsigned int seed=10;
    if(__core_ID() == 4)
        seed = 12;
    for(i=0;i<100000;i++)
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

    printf("Default FC Frequency      = %d Hz\n", FLL_GetFrequency(uFLL_SOC));
    printf("voltage: %lu\n", current_voltage());

    /* Set voltage */
    set_voltage_return = PMU_SetVoltage(1000,0);
    if (set_voltage_return) {
        printf("Failed to change voltage! Code: %d\n",set_voltage_return);
    }
    printf("Voltage: %lu - FCMaxFreq: %d - FCMaxFreq: %d\n", current_voltage(),FLL_SoCMaxFreqAtV(current_voltage()),FLL_ClusterMaxFreqAtV(current_voltage()));

    /* Set frequency */
    if (FLL_SetFrequency(uFLL_SOC, 100000000 , 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }
    printf("FC Freq: %d Hz - Voltage: %lu mV\n", FLL_GetFrequency(uFLL_SOC),current_voltage());
    random_gen(NULL);

    unsigned long begin_ms = us_ticker_read();

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    /* Set frequency */
    if (FLL_SetFrequency(uFLL_CLUSTER, 200000000 , 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    }
    printf("Cluster Freq: %d Hz - Voltage: %lu mV\n", FLL_GetFrequency(uFLL_CLUSTER),current_voltage());

    /* FC send a task to Cluster */
    CLUSTER_SendTask(0, Master_Entry, 0, 0);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    unsigned long end_ms = us_ticker_read();
    printf("elapsed: %lu us\n", end_ms-begin_ms);


    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    exit(error);
}
