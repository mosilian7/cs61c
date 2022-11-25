#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "transpose.h"

void benchmark(int *A, int *B, int n, int blocksize,
    void (*transpose)(int, int, int*, int*), char *description) {

    int i, j;
    printf("Testing %s: ", description);

    /* initialize A,B to random integers */
    srand48( time( NULL ) );
    for( i = 0; i < n*n; i++ ) A[i] = lrand48( );
    for( i = 0; i < n*n; i++ ) B[i] = lrand48( );

    /* measure performance */
    struct timeval start, end;

    gettimeofday( &start, NULL );
    transpose( n, blocksize, B, A );
    gettimeofday( &end, NULL );

    double seconds = (end.tv_sec - start.tv_sec) +
        1.0e-6 * (end.tv_usec - start.tv_usec);
    printf( "%g milliseconds\n", seconds*1e3 );

    /* check correctness */
    for( i = 0; i < n; i++ ) {
        for( j = 0; j < n; j++ ) {
            if( B[j+i*n] != A[i+j*n] ) {
                printf("Error!!!! Transpose does not result in correct answer!!\n");
                exit( -1 );
            }
        }
    }
}

void sanity_test(void (*transpose)(int, int, int*, int*), char* description) {
    int src[25];
    for (int i = 0; i < 25; i += 1) {
        src[i] = i;
    }
    int expect[25]  = { 0,5,10,15,20,1,6,11,16,21,2,7,12,17,22,3,8,13,18,23,4,9,14,19,24 };
    int dst[25] ;
    transpose(5, 2, dst, src);
    for (int i = 0; i < 25; i += 1) {
        if (dst[i] != expect[i]) {
            printf("Sanity test failed on %s\n", description);
            exit(1);
        }
    }
    transpose(5, 3, dst, src);
    for (int i = 0; i < 25; i += 1) {
        if (dst[i] != expect[i]) {
            printf("Sanity test failed on %s\n", description);
            exit(1);
        }
    }
    transpose(5, 5, dst, src);
    for (int i = 0; i < 25; i += 1) {
        if (dst[i] != expect[i]) {
            printf("Sanity test failed on %s\n", description);
            exit(1);
        }
    }
    printf("Sanity test pass on %s\n", description);
    
}

int main( int argc, char **argv ) {
    if (argc != 3) {
        printf("Usage: transpose <n> <blocksize>\nExiting.\n");
        exit(1);
    }

    //sanity_test(transpose_naive, "naive");
    //sanity_test(transpose_blocking, "blocking");

    int n = atoi(argv[1]);
    int blocksize = atoi(argv[2]);

    /* allocate an n*n block of integers for the matrices */
    int *A = (int*)malloc( n*n*sizeof(int) );
    int *B = (int*)malloc( n*n*sizeof(int) );

    /* run tests */
    benchmark(A, B, n, blocksize, transpose_naive, "naive transpose");
    benchmark(A, B, n, blocksize, transpose_blocking, "transpose with blocking");

    /* release resources */
    free( A );
    free( B );
    return 0;
}
