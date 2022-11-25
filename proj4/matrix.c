#include "matrix.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Below are some intel intrinsics that might be useful
 * void _mm256_storeu_pd (double * mem_addr, __m256d a)
 * __m256d _mm256_set1_pd (double a)
 * __m256d _mm256_set_pd (double e3, double e2, double e1, double e0)
 * __m256d _mm256_loadu_pd (double const * mem_addr)
 * __m256d _mm256_add_pd (__m256d a, __m256d b)
 * __m256d _mm256_sub_pd (__m256d a, __m256d b)
 * __m256d _mm256_fmadd_pd (__m256d a, __m256d b, __m256d c)
 * __m256d _mm256_mul_pd (__m256d a, __m256d b)
 * __m256d _mm256_cmp_pd (__m256d a, __m256d b, const int imm8)
 * __m256d _mm256_and_pd (__m256d a, __m256d b)
 * __m256d _mm256_max_pd (__m256d a, __m256d b)
*/


/*
 * Generates a random double between `low` and `high`.
 */
double rand_double(double low, double high) {
    double range = (high - low);
    double div = RAND_MAX / range;
    return low + (rand() / div);
}

/*
 * Generates a random matrix with `seed`.
 */
void rand_matrix(matrix *result, unsigned int seed, double low, double high) {
    srand(seed);
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            set(result, i, j, rand_double(low, high));
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by the double pointer mat with
 * `rows` rows and `cols` columns. You should also allocate memory for the data array
 * and initialize all entries to be zeros. Remember to set all fieds of the matrix struct.
 * `parent` should be set to NULL to indicate that this matrix is not a slice.
 * You should return -1 if either `rows` or `cols` or both have invalid values, or if any
 * call to allocate memory in this function fails. If you don't set python error messages here upon
 * failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix(matrix **mat, int rows, int cols) {
    /* TODO: YOUR CODE HERE */
    if (rows <= 0 || cols <= 0) {
        fprintf(stderr, "allocate_matrix: invalid argument \n");
        return -1;
    }
    *mat = malloc(sizeof(matrix));
    if (NULL == *mat) {
        fprintf(stderr, "malloc failed \n");
        return -1;
    }
    (*mat)->rows = rows;
    (*mat)->cols = cols;
    (*mat)->is_1d = (rows == 1 || cols == 1) ? 1 : 0;
    (*mat)->ref_cnt = 1;
    (*mat)->parent = NULL;
    
    int len = sizeof(double*) * rows + sizeof(double) * rows * cols;
    //(*mat)->data = malloc(rows * sizeof(double*));
    (*mat)->data = malloc(len);
    if (NULL == (*mat)->data) {
        fprintf(stderr, "malloc failed \n");
        return -1;
    }

    double* ptr = (double*)((*mat)->data + rows);

    double* mat_row;
    __m256d _zero = _mm256_setzero_pd();

    for (int i = 0; i < rows; i += 1) {
        //(*mat)->data[i] = malloc(cols * sizeof(double));
        //(*mat)->data[i] = aligned_alloc(32, 32 * ((cols * sizeof(double) / 32) + 1));
        (*mat)->data[i] = ptr + cols * i;
        /*if (NULL == (*mat)->data[i]) {
            fprintf(stderr, "malloc failed \n");
            return -1;
        }*/
        mat_row = (*mat)->data[i];
        for (int j = 0; j < cols / 4 * 4; j += 4) {
            _mm256_storeu_pd(mat_row + j, _zero);
        }
        // tail case
        for (int j = cols / 4 * 4; j < cols; j += 1) {
            mat_row[j] = 0.0;
        }
    }
    return 0;
}

/*
 * Allocate space for a matrix struct pointed to by `mat` with `rows` rows and `cols` columns.
 * This is equivalent to setting the new matrix to be
 * from[row_offset:row_offset + rows, col_offset:col_offset + cols]
 * If you don't set python error messages here upon failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix_ref(matrix **mat, matrix *from, int row_offset, int col_offset,
                        int rows, int cols) {
    /* TODO: YOUR CODE HERE */
    if (rows <= 0 || cols <= 0 || rows + row_offset > from->rows || cols + col_offset > from->cols) {
        fprintf(stderr, "allocate_matrix_ref: invalid argument \n");
        return -1;
    }
    *mat = malloc(sizeof(matrix));
    if (NULL == *mat) {
        fprintf(stderr, "malloc failed \n");
        return -1;
    }
    (*mat)->rows = rows;
    (*mat)->cols = cols;
    (*mat)->is_1d = (rows == 1 || cols == 1) ? 1 : 0;

    matrix* curr = from;
    while (curr->parent != NULL) {
        curr = curr->parent;
    }
    (*mat)->parent = curr;

    (*mat)->parent->ref_cnt += 1; // WARNING: how to handle ref_cnt?
    (*mat)->ref_cnt = 1; // WARNING: how to handle ref_cnt?
    (*mat)->data = malloc(rows * sizeof(double*));
    if (NULL == (*mat)->data) {
        fprintf(stderr, "malloc failed \n");
        return -1;
    }
    for (int i = 0; i < rows; i += 1) {
        (*mat)->data[i] = from->data[i + row_offset] + col_offset;
    }
    return 0;
}

/*
 * This function will be called automatically by Python when a numc matrix loses all of its
 * reference pointers.
 * You need to make sure that you only free `mat->data` if no other existing matrices are also
 * referring this data array.
 * See the spec for more information.
 */
void deallocate_matrix(matrix *mat) {
    /* TODO: YOUR CODE HERE */
    if (mat == NULL)
        return;
    mat->ref_cnt -= 1;
    if (mat->ref_cnt <= 0) {
        if (mat->parent == NULL) {
            for (int i = 0; i < mat->rows; i += 1) {
                //free(mat->data[i]);
            }
        } else {
            mat->parent->ref_cnt -= 1;
            if (mat->parent->ref_cnt <= 0)
                deallocate_matrix(mat->parent);
        }
        free(mat->data);
        free(mat);     
    }
}

/*
 * Return the double value of the matrix at the given row and column.
 * You may assume `row` and `col` are valid.
 */
double get(matrix *mat, int row, int col) {
    /* TODO: YOUR CODE HERE */
    if (row < 0 || col < 0 || row >= mat->rows || col >= mat->cols) {
        fprintf(stderr, "get: invalid argument \n");
    }
    return mat->data[row][col];
}

/*
 * Set the value at the given row and column to val. You may assume `row` and
 * `col` are valid
 */
void set(matrix *mat, int row, int col, double val) {
    /* TODO: YOUR CODE HERE */
    if (row < 0 || col < 0 || row >= mat->rows || col >= mat->cols) {
        fprintf(stderr, "set: invalid argument \n");
    }
    mat->data[row][col] = val;
}

/*
 * Set all entries in mat to val
 */
void fill_matrix(matrix *mat, double val) {
    /* TODO: YOUR CODE HERE */
    double* mat_row;
    __m256d tmp1;
    int i = 0, j = 0;
    #pragma omp parallel for private(j, mat_row, tmp1)
    for (i = 0; i < mat->rows; i += 1) {
        mat_row = mat->data[i];
        for (j = 0; j < mat->cols / 4 * 4; j += 4) {
            tmp1 = _mm256_set1_pd(val);
            _mm256_storeu_pd(mat_row + j, tmp1);
        }
        // tail case
        for (j = mat->cols / 4 * 4; j < mat->cols; j += 1) {
            mat_row[j] = val;
        }
    }
}

/*
 * Store the result of adding mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int add_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    __m256d tmp1, tmp2;
    double *mat1_row, *mat2_row, *result_row;
    int i = 0, j = 0;
    #pragma omp parallel for private(j, mat1_row, mat2_row, result_row, tmp1, tmp2)
    for (i = 0; i < mat1->rows; i += 1) {
        mat1_row = mat1->data[i];
        mat2_row = mat2->data[i];
        result_row = result->data[i];
        for (j = 0; j < mat1->cols / 4 * 4; j += 4) {
            tmp1 = _mm256_loadu_pd(mat1_row + j);
            tmp2 = _mm256_loadu_pd(mat2_row + j);
            tmp1 = _mm256_add_pd(tmp1, tmp2);
            _mm256_storeu_pd(result_row + j, tmp1);
        }
        //tail case
        for (j = mat1->cols / 4 * 4; j < mat1->cols; j += 1) {
            result_row[j] = mat1_row[j] + mat2_row[j];
        }
    }
    return 0;
}
/* plain loop:
for (int i = 0; i < mat1->rows; i += 1) {
        for (int j = 0; j < mat1->cols; j += 1) {
            result->data[i][j] = mat1->data[i][j] + mat2->data[i][j];
        }
    }*/

/*
 * Store the result of subtracting mat2 from mat1 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int sub_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    __m256d tmp1, tmp2;
    double *mat1_row, *mat2_row, *result_row;
    int i=0, j=0;
    #pragma omp parallel for private(j, mat1_row, mat2_row, result_row, tmp1, tmp2)
    for (i = 0; i < mat1->rows; i += 1) {
        mat1_row = mat1->data[i];
        mat2_row = mat2->data[i];
        result_row = result->data[i];
        for (j = 0; j < mat1->cols / 4 * 4; j += 4) {
            tmp1 = _mm256_loadu_pd(mat1_row + j);
            tmp2 = _mm256_loadu_pd(mat2_row + j);
            tmp1 = _mm256_sub_pd(tmp1, tmp2);
            _mm256_storeu_pd(result_row + j, tmp1);
        }
        //tail case
        for (j = mat1->cols / 4 * 4; j < mat1->cols; j += 1) {
            result_row[j] = mat1_row[j] - mat2_row[j];
        }
    }
    return 0;
}

/*
 * Store the result of multiplying mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that matrix multiplication is not the same as multiplying individual elements.
 */
int mul_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    /* TODO: YOUR CODE HERE */
    double *result_row, *mat1_row, *mat2_row;
    __m256d tmp1, tmp2, mat1_ik;
    int i=0, j=0, k=0;
    #pragma omp parallel for private(k, j, result_row, mat1_row, mat2_row, tmp1, tmp2, mat1_ik)
    for(i = 0; i < mat1->rows; i++ ) {
        result_row = result->data[i];
        mat1_row = mat1->data[i];
        for(k = 0; k < mat1->cols; k++ ) {
            mat2_row = mat2->data[k];
            mat1_ik = _mm256_set1_pd(mat1->data[i][k]);
            for(j = 0; j < mat2->cols / 4 * 4; j += 4 ) {
                tmp1 = _mm256_loadu_pd(mat2_row + j);
                tmp2 = _mm256_loadu_pd(result_row + j);
                tmp1 = _mm256_fmadd_pd(mat1_ik, tmp1, tmp2);
                _mm256_storeu_pd(result_row + j, tmp1);
            }
            // tail case
            for(j = mat2->cols / 4 * 4; j < mat2->cols; j++ ) {
                result_row[j] += mat1_row[k] * mat2_row[j];
            }
        }
    }
    return 0;
}
/* plain loop:
for(int i = 0; i < mat1->rows; i++ ) {
        for(int k = 0; k < mat1->cols; k++ ) {
            for(int j = 0; j < mat2->cols; j++ ) {
                result->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
            }
        }
    }*/

/*
 * Store the result of raising mat to the (pow)th power to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that pow is defined with matrix multiplication, not element-wise multiplication.
 */
int pow_matrix(matrix *result, matrix *mat, int pow) {
    /* TODO: YOUR CODE HERE */
    if (pow == 0) {
        for (int i = 0; i < mat->rows; i += 1) {
            result->data[i][i] = 1.0;
        }
        return 0;
    }
    _cpy_matrix(result, mat);
    matrix* tmp;
    allocate_matrix(&tmp, mat->rows, mat->cols);
    while (pow > 1) {
        _cpy_matrix(tmp, result);
        fill_matrix(result, 0);
        mul_matrix(result, mat, tmp);
        pow -= 1;
    }
    deallocate_matrix(tmp);
    return 0;
}

/*
 * Store the result of element-wise negating mat's entries to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int neg_matrix(matrix *result, matrix *mat) {
    /* TODO: YOUR CODE HERE */
    __m256d tmp1;
    __m256d _zero = _mm256_setzero_pd();
    double *mat_row, *result_row;
    int i=0, j=0;
    #pragma omp parallel for private(j, tmp1, mat_row, result_row)
    for (i = 0; i < mat->rows; i += 1) {
        mat_row = mat->data[i];
        result_row = result->data[i];
        for (j = 0; j < mat->cols / 4 * 4; j += 4) {
            tmp1 = _mm256_loadu_pd(mat_row + j);
            tmp1 = _mm256_sub_pd(_zero, tmp1);
            _mm256_storeu_pd(result_row + j, tmp1);
        }
        //tail case
        for (j = mat->cols / 4 * 4; j < mat->cols; j += 1) {
            result_row[j] = -mat_row[j];
        }
    }
    return 0;
}

/*
 * Store the result of taking the absolute value element-wise to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int abs_matrix(matrix *result, matrix *mat) {
    /* TODO: YOUR CODE HERE */
    /*
    for (int i = 0; i < mat->rows; i += 1) {
        for (int j = 0; j < mat->cols; j += 1) {
            result->data[i][j] = mat->data[i][j] > 0 ? mat->data[i][j] : -mat->data[i][j];
        }
    }
    return 0;*/
    __m256d tmp1, tmp2;
    __m256d _zero = _mm256_setzero_pd();
    double *mat_row, *result_row;
    int i=0, j=0;
    #pragma omp parallel for private(j, tmp1, mat_row, result_row)
    for (i = 0; i < mat->rows; i += 1) {
        mat_row = mat->data[i];
        result_row = result->data[i];
        for (j = 0; j < mat->cols / 4 * 4; j += 4) {
            tmp1 = _mm256_loadu_pd(mat_row + j);
            tmp2 = _mm256_cmp_pd(tmp1, _zero, 14); //_CMP_GT_OS
            tmp2 = _mm256_and_pd(tmp2, tmp1);
            tmp1 = _mm256_sub_pd(tmp2, tmp1);
            tmp1 = _mm256_or_pd(tmp2, tmp1);
            _mm256_storeu_pd(result_row + j, tmp1);
        }
        //tail case
        for (j = mat->cols / 4 * 4; j < mat->cols; j += 1) {
            result_row[j] = mat_row[j] > 0 ? mat_row[j] : -mat_row[j];
        }
    }
    return 0;
}

int _cpy_matrix(matrix *result, matrix *mat) {
    if (result->rows != mat->rows || result->cols != mat->cols) {
        fprintf(stderr, "_cpy_matrix: invalid dimension \n");
        return -1;
    }
    /*
    for (int i = 0; i < mat->rows; i += 1) {
        for (int j = 0; j < mat->cols; j += 1) {
            result->data[i][j] = mat->data[i][j];
        }
    }
    return 0;*/
    __m256d tmp1;
    double *mat_row, *result_row;
    int i=0, j=0;
    #pragma omp parallel for private(j, tmp1, mat_row, result_row)
    for (i = 0; i < mat->rows; i += 1) {
        mat_row = mat->data[i];
        result_row = result->data[i];
        for (j = 0; j < mat->cols / 4 * 4; j += 4) {
            tmp1 = _mm256_loadu_pd(mat_row + j);
            _mm256_storeu_pd(result_row + j, tmp1);
        }
        //tail case
        for (j = mat->cols / 4 * 4; j < mat->cols; j += 1) {
            result_row[j] = mat_row[j];
        }
    }
    return 0;
}

