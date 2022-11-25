#include "transpose.h"
#include <math.h>
#include <stdio.h>
#define MIN(x, y) (x > y ? y : x)

void transpose_at(int n, int blocksize, int* dst, int* src, int block_x, int block_y);

/* The naive transpose function as a reference. */
void transpose_naive(int n, int blocksize, int *dst, int *src) {
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < n; y++) {
            dst[y + x * n] = src[x + y * n];
        }
    }
}

/* Implement cache blocking below. You should NOT assume that n is a
 * multiple of the block size. */
void transpose_blocking(int n, int blocksize, int *dst, int *src) {
    int block_dim = (int)ceil((double)n / (double)blocksize);

    //printf("%d\n", block_dim);

    for (int block_x = 0; block_x < block_dim; block_x += 1) {
        for (int block_y = 0; block_y < block_dim; block_y += 1) {
            for (int x = block_x * blocksize; x < MIN((block_x + 1) * blocksize, n); x += 1) {
                for (int y = block_y * blocksize; y < MIN((block_y + 1) * blocksize, n); y += 1) {
                    //printf("x: %d, y: %d\n", x, y);
                    dst[y + x * n] = src[x + y * n];
                }
            }
        }
    }
}

void transpose_at(int n, int blocksize, int* dst, int* src, int block_x, int block_y) {
    for (int x = block_x * blocksize; x < MIN((block_x + 1) * blocksize, n); x += 1) {
        for (int y = block_y * blocksize; x < MIN((block_y + 1) * blocksize, n); y += 1) {
            dst[y + x * n] = src[x + y * n];
        }
    }
}
