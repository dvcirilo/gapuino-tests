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

void initialize_seeds(unsigned int *L1_mem, int core_num, unsigned int seed){
    for (int i = 0; i < core_num; i++) {
       L1_mem[i] = seed; 
    }
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

void test_rand(int frequency, int voltage)
{
    int set_voltage_return, error=0, time_ms;
    int success_counter=0, failure_counter=0;
    int total_time=0, calls=0, call_total=0;

    unsigned int *L1_mem = L1_Malloc(CORE_NUMBER*sizeof(unsigned int));

    set_voltage_return = PMU_SetVoltage(voltage,0);

    if (set_voltage_return) {
        printf("Failed to change voltage! Code: %d\n",set_voltage_return);
    }
    printf("Voltage: %lu - FCMaxFreq: %d kHz - ClusterMaxFreq: %d kHz\n",
            current_voltage(),FLL_SoCMaxFreqAtV(current_voltage())/F_DIV,
            FLL_ClusterMaxFreqAtV(current_voltage())/F_DIV);

    /* Set frequency */
    if (FLL_SetFrequency(uFLL_CLUSTER, frequency, 0) == -1) {
        printf("Error of changing frequency, check Voltage value!\n");
    } else {
        printf("Set: Cluster Freq: %d kHz - Voltage: %lu mV\n",
                FLL_GetFrequency(uFLL_CLUSTER)/F_DIV,current_voltage());
    }

    /* Resets and initialize TIMER (on FC) */
    Timer_Initialize(TIMER, 0);
    Timer_Enable(TIMER);

    /* Runs NUM_TESTS tests. Each test with RUNS calls to random_gen() */
    for(int j=0;j<NUM_TESTS;j++) {
        initialize_seeds(L1_mem, CORE_NUMBER, (unsigned int) SEED);
        for(int i=0;i<RUNS;i++) {
            CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
            CLUSTER_Wait(0);
            calls++;
            if (L1_mem[CORE_NUMBER-1]^rand_values[i]){
                time_ms = (int)(Timer_ReadCycle(TIMER) 
                            / (FLL_GetFrequency(uFLL_SOC)/1000));
                total_time += time_ms;
                failure_counter++;
                Timer_Disable(TIMER);
                printf("%d,%d,0x%08x,0x%08x,%d,%d\n", time_ms, calls,
                        L1_mem[CORE_NUMBER-1], rand_values[i],
                        failure_counter,success_counter);
                call_total += calls;
                calls = 0;
                Timer_Initialize(TIMER, 0);
                Timer_Enable(TIMER);
                break;
            } else {
                success_counter++;
            }
        }
    }
    call_total += calls;
    time_ms = (int)(Timer_ReadCycle(TIMER) 
                / (FLL_GetFrequency(uFLL_SOC)/1000));
    total_time += time_ms;
    Timer_Disable(TIMER);
    printf("success: %d - fail: %d - total: %d - total time: %dms\n",
            success_counter, failure_counter, call_total, total_time);
}

int main()
{
    /* Initialize FC Clock */
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 1);

    printf("FC Frequency: %d kHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, current_voltage());

    /* Create a vector on L2 with 10k values of rand */
    unsigned int rand_var = (unsigned int) SEED;
    for (int i = 0; i < RUNS; i++) {
        rand_values[i] = rand_r(&rand_var);
    }

    printf("Done generating %dk items table for seed = %d\n", RUNS/1000, SEED);

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);


    test_rand(226000000, 1000);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    printf("Test finished\n");

    exit(0);
}
