// PRINTF
#include "gap_common.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include "test.h"

unsigned int rand_values[RUNS];

void random_gen(void *arg)
{
    unsigned int *L1_mem = (unsigned int *)arg;
    rand_r(&L1_mem[__core_ID()]);
}

void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(random_gen, (void *) L1_mem);
}

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[
            READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}

void test_rand(unsigned int *L1_mem, int frequency, int voltage)
{
    int set_voltage_return, error=0, time_ms;
    int success_counter=0, failure_counter=0;
    int total_time=0;

    set_voltage_return = PMU_SetVoltage(voltage,0);

    if (set_voltage_return) {
        printf("Failed to change voltage! Code: %d\n",set_voltage_return);
    }
    printf("Voltage: %lu - FCMaxFreq: %d MHz - ClusterMaxFreq: %d MHz\n",
            current_voltage(),FLL_SoCMaxFreqAtV(current_voltage())/F_DIV,
            FLL_ClusterMaxFreqAtV(current_voltage())/F_DIV);

    /* Set frequency */
    if (FLL_SetFrequency(uFLL_CLUSTER, frequency, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    } else {
        printf("Set: Cluster Freq: %d MHz - Voltage: %lu mV\n",
                FLL_GetFrequency(uFLL_CLUSTER)/F_DIV,current_voltage());
    }

    __disable_irq();
    Timer_Initialize(TIMER, 0);
    Timer_Enable(TIMER);
    /* Runs NUM_TESTS tests. Each test with RUNS calls to random_gen() */
    for(int j=0;j<NUM_TESTS;j++) {
        for(int i=0;i<RUNS;i++) {
            CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
            CLUSTER_Wait(0);
            if (L1_mem[CORE_NUMBER-1]^rand_values[i]){
                time_ms = (int)(Timer_ReadCycle(TIMER) 
                            / (FLL_GetFrequency(uFLL_SOC)/1000));
                total_time += time_ms;
                failure_counter++;
                Timer_Disable(TIMER);
                printf("%d,%d,0x%08x,0x%08x,%d,%d\n", time_ms, i,
                        L1_mem[CORE_NUMBER-1], rand_values[i],
                        failure_counter,success_counter);
                error = 1;
                Timer_Initialize(TIMER, 0);
                Timer_Enable(TIMER);
                break;
            } else {
                error = 0; 
            }
        }
        if(!error){
            time_ms = (int)(Timer_ReadCycle(TIMER)
                        / (FLL_GetFrequency(uFLL_SOC)/1000));
            total_time += time_ms;
            success_counter++;
            error = 0;
        }
        L1_mem[CORE_NUMBER-1] = (unsigned int) SEED;
    }
    Timer_Disable(TIMER);
    __enable_irq();
    printf("success: %d - fail: %d - total time: %dms\n",
            success_counter, failure_counter, total_time);
}

int main()
{
    /*Initialize FC Clock*/
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 1);

    printf("FC Frequency: %d MHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, current_voltage());

    /*Create a vector on L2 with 10k values of rand */
    unsigned int rand_var = (unsigned int) SEED;
    for (int i = 0; i < RUNS; i++) {
        rand_values[i] = rand_r(&rand_var);
    }

    printf("Done generating table for seed = %d\n", SEED);

    /*
     *printf("before\n");
     *Timer_Initialize(TIMER, 0);
     *Timer_Enable(TIMER);
     *wait(10);
     *printf("after %d ms\n", 
                (int)(Timer_ReadCycle(TIMER)
                    / (FLL_GetFrequency(uFLL_SOC)/1000)));
     *Timer_Disable(TIMER);
     */

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    unsigned int *L1_mem = L1_Malloc(CORE_NUMBER*sizeof(unsigned int));

    for (int i = 0; i < CORE_NUMBER; i++) {
       L1_mem[i] = (unsigned int) SEED; 
    }

    test_rand(L1_mem, 225000000, 1000);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    exit(error);
}
