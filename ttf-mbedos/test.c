// PRINTF
#include "gap_common.h"

// FEATURE_CLUSTER
#include "gap_cluster.h"
#include "gap_dmamchan.h"
#include <stdlib.h>
#include <time.h>
#include "test.h"

void random_gen(void *arg)
{
    int *L1_mem = (int *)arg;
    unsigned int seed=SEED;
    rand_r(&seed);
    L1_mem[__core_ID()] = rand_r(&seed);
    /*printf("%d\t%d\n", __core_ID(), L1_mem[__core_ID()]);*/
}

void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(random_gen, (void *) L1_mem);
}

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}

void test_frequency(int *L1_mem)
{
    for (int frequency = F_MIN; frequency <= F_MAX; frequency+=F_STEP) {
        /* Set frequency */
        if (FLL_SetFrequency(uFLL_CLUSTER, frequency, 0) == -1) {
            printf("Error of changing frequency, check Voltage value!\n");
        }

        /* FC send a task to Cluster */
        for(int i=0;i<RUNS;i++) {
            CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
            CLUSTER_Wait(0);
            if (L1_mem[0]^L1_mem[1]^L1_mem[2]^L1_mem[3]^L1_mem[4]^L1_mem[5]^L1_mem[6]^L1_mem[7]){
                printf("Diff at: Cluster Freq: %d MHz - Voltage: %lu mV\n",
                        FLL_GetFrequency(uFLL_CLUSTER)/F_DIV,current_voltage());
                break;
            } else {
                /*printf("Pass %d\n", i);*/
            
            }
        }
    }
}

void test_voltage(int *L1_mem)
{
    int set_voltage_return;
    for (int i = V_MAX; i>=V_MIN; i-=V_STEP){
        set_voltage_return = PMU_SetVoltage(i,0);

        if (set_voltage_return) {
            printf("Failed to change voltage! Code: %d\n",set_voltage_return);
        }

        printf("Voltage: %lu - FCMaxFreq: %d MHz - FCMaxFreq: %d MHz\n",
                current_voltage(),FLL_SoCMaxFreqAtV(current_voltage())/F_DIV,
                FLL_ClusterMaxFreqAtV(current_voltage())/F_DIV);
        test_frequency(L1_mem);
    }
}

int main()
{
    int set_voltage_return;
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 1);
    printf("FC Frequency: %d MHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, current_voltage());

    random_gen(NULL);

    clock_t begin = clock();

    /* Cluster Start - Power on */
    CLUSTER_Start(0, CORE_NUMBER);

    int *L1_mem = L1_Malloc(8);

    test_frequency(L1_mem);

    printf("elapsed: %d us\n", (clock()-begin)/CLOCKS_PER_SEC);

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);



    /* Check read values */
    int error = 0;

    if (error) printf("Test failed with %d errors\n", error);
    else printf("Test success\n");

    exit(error);
}
