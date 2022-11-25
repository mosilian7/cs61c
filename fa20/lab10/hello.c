#include <stdio.h>
#include <omp.h>
int omp_thread_count() {
    int n = 0;
    #pragma omp parallel reduction(+:n)
    {
		n = 1;
	}
    return n;
}
int main() {
	/*#pragma omp parallel
	{
		int thread_ID = omp_get_thread_num();
		printf(" hello world %d\n", thread_ID);
	}*/
	int num_threads = omp_thread_count();
	printf("%d\n", num_threads);
}
