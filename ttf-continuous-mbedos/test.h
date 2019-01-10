#define FC_FREQ       (150000000)
#define CORE_NUMBER   (1)
#define SEED          (10)
#define V_MAX         (1200)
#define V_MIN         (1000)
#define V_STEP        (50)
#define F_MAX         (350000000)
#define F_MIN         (250000000)
#define F_STEP        (5000000)
#define F_DIV         (1000)
#define RUNS          (100000)
#define NUM_TESTS     (100)

#define TIMER TIMER1

void random_gen(void *arg);

void Master_Entry(int *L1_mem);

uint32_t current_voltage(void);

void test_rand(int frequency, int voltage);

