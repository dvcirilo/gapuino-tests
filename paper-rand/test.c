#include "test.h"
#include "functions.h"

/* Reference rand values on L2 */
unsigned int rand_values[RUNS];

/* Generate a random number */
void random_gen(void *arg)
{
    unsigned int *L1_mem = (unsigned int *)arg;
    rand_r(&L1_mem[__core_ID()]);
}

/* Initialize seeds */
void initialize_seeds(unsigned int *L1_mem, int core_num, unsigned int seed)
{
    for (int i = 0; i < core_num; i++) {
       L1_mem[i] = seed; 
    }
}

/* Master entry in charge of spawning the parallel function to all the cores */
void Master_Entry(int * L1_mem)
{
    CLUSTER_CoresFork(random_gen, (void *) L1_mem);
}

/* The actual test. */
struct run_info test_rand(bool verbose)
{
    int error=0, time_ms=0;
    int success_counter=0, failure_counter=0;
    int total_time=0, calls=0, call_total=0;
    struct run_info runs;

    unsigned int *L1_mem = L1_Malloc(CORE_NUMBER*sizeof(unsigned int));

    /* Runs NUM_TESTS tests. Each test with RUNS calls to random_gen() */
    for(int j=0;j<NUM_TESTS;j++) {
        initialize_seeds(L1_mem, CORE_NUMBER, (unsigned int) SEED);
        for(int i=0;i<RUNS;i++) {
            CLUSTER_SendTask(0, Master_Entry, (void *) L1_mem, 0);
            CLUSTER_Wait(0);
            calls++;
            if (L1_mem[CORE_NUMBER-1]^rand_values[i]){
                failure_counter++;
                if (verbose)
                    printf("%d,%d,0x%08x,0x%08x,%d,%d\n", time_ms, calls,
                            L1_mem[CORE_NUMBER-1], rand_values[i],
                            failure_counter,success_counter);
                call_total += calls;
                calls = 0;
                break;
            } else {
                success_counter++;
            }
        }
    }
    call_total += calls;
    runs.success_counter = success_counter;
    runs.failure_counter = failure_counter;
    runs.call_total = call_total;
    runs.total_time = 0;
    L1_Free(L1_mem, CORE_NUMBER*sizeof(unsigned int));
    return runs;
}

static GPIO_Type *const gpio_addrs[] = GPIO_BASE_PTRS;
static PORT_Type *const port_addrs[] = PORT_BASE_PTRS;

int main()
{
    PinName led = GPIO_A17;

    struct run_info runs;

    /* Parsing GPIO pin to get real number for port, gpio and pin*/
    uint32_t port_number = GET_GPIO_PORT(led);
    uint32_t gpio_number = GET_GPIO_NUM(led);
    uint32_t pin_number  = GET_GPIO_PIN_NUM(led);

    PORT_SetPinMux(port_addrs[port_number], pin_number,  uPORT_MuxGPIO);

    /* Init GPIO - OUTPUT. */
    gpio_pin_config_t gpio_config = { .pinDirection  = uGPIO_DigitalOutput,
                                      .outputLogic   = uGPIO_LOW,
                                      .pullSelect    = uGPIO_PullUpEnable,
                                      .driveStrength = uGPIO_LowDriveStrength,
                                    };

    GPIO_PinInit ( gpio_addrs[port_number], gpio_number, &gpio_config );

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


    printf("run,freq,success,fail,total,time\n");
    int time = 100;
    int fmax, fstep, freq = 210000000;
    fmax = 230000000;
    fstep = 1000000;
    while (freq <= fmax) {
        freq = freq + fstep;
        set_voltage_current(freq, 1000, false);
        for (int i = 0; i < 100; i++) {
            // set trigger
            GPIO_WritePinOutput( gpio_addrs[port_number], gpio_number, uGPIO_HIGH );

            runs = test_rand(false);

            // unset trigger
            GPIO_WritePinOutput( gpio_addrs[port_number], gpio_number, uGPIO_LOW );
            printf("%d,%d,%d,%d,%d\n",
                    i,freq,runs.success_counter, runs.failure_counter,
                    runs.call_total);

             /*Test for osDelay()*/
            time = 100;
            while (time) {
                osDelay(time--);
            }
        }
    }

    /* Cluster Stop - Power down */
    CLUSTER_Stop(0);

    printf("Test finished\n");

    exit(0);
}
