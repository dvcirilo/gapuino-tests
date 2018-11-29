// PRINTF
#include "gap_common.h"
#include <stdlib.h>

typedef struct DataRet {
	int time;
	int calls;
};

int main()
{
	struct DataRet *data = malloc(100*sizeof(struct DataRet));
	printf("%p\n", data);
	int *test = malloc(10000*sizeof(int));
	printf("%p\n", test);

	for(int i=0;i<100;i++)
		test[i] = i;

	for(int i=0;i<100;i++)
		printf("%p - %d\n", &test[i], test[i]);

	exit(0);
}
