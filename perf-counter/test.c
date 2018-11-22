#include <stdlib.h>
#include "gap_common.h"
#include "gap_performance.h"

performance_t perf;

#define R_RUNS    (100000)
#define P_RUNS    (1000)
#define SEED      (10)
#define FC_FREQ   (100000000)
#define F_DIV     (1000000)
#define PERF_Msk PCER_INSTR_Msk
#define PERF_Pos PCER_INSTR_Pos

void random_gen(void)
{
    srand(SEED);
    for(int i=0;i<R_RUNS;i++)
        rand();
}

uint32_t current_voltage(void)
{
    return DCDC_TO_mV(PMU_State.DCDC_Settings[READ_PMU_REGULATOR_STATE(PMU_State.State)]);
}

int main()
{
    FLL_SetFrequency(uFLL_SOC, FC_FREQ, 1);
    printf("FC Frequency: %d MHz - Voltage: %lu mV\n",
            FLL_GetFrequency(uFLL_SOC)/F_DIV, current_voltage());

    for (int i = 0; i < P_RUNS; i++) {
        __disable_irq();
        PERFORMANCE_Start(&perf, PERF_Msk);
        random_gen();
        PERFORMANCE_Stop(&perf);
        __enable_irq();
        printf("%d\n" , (int)PERFORMANCE_Get(&perf, PERF_Pos));
    }

    exit(0);
}
