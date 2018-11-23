#define FC_FREQ       (100000000)
#define CORE_NUMBER   (8)
#define SEED          (10)
#define V_MAX         (1200)
#define V_MIN         (1000)
#define V_STEP        (50)
#define F_MAX         (350000000)
#define F_MIN         (250000000)
#define F_STEP        (5000000)
#define F_DIV         (1000000)
#define RUNS          (10)
#define NUM_TESTS     (100)

void random_gen(void *arg);

void Master_Entry(int *L1_mem);

uint32_t current_voltage(void);

void test_frequency(int *L1_mem);

void test_voltage(int *L1_mem);