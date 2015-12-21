/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

/*******************************************
 Name: Jiang Jiheng Student No.: 5130379033

 Lab 8 Part B: Matrix transpose
*******************************************/

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*******************************************
 case 32*32:
 step = 8 because the block size is 32
 so it can store 8 ints a time
*******************************************/
void test32(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, t1, t2, t3, t4, t5, t6, t7, t8;

	if (M == 32 && N == 32) {
		for (j = 0; j < M; j += 8) {
			for (i = 0; i < N; i++) {
				t1 = A[i + 0][j + 0];
				t2 = A[i + 0][j + 1];
				t3 = A[i + 0][j + 2];
				t4 = A[i + 0][j + 3];
				t5 = A[i + 0][j + 4];
				t6 = A[i + 0][j + 5];
				t7 = A[i + 0][j + 6];
				t8 = A[i + 0][j + 7];
				
				B[j + 0][i + 0] = t1;
				B[j + 1][i + 0] = t2;
				B[j + 2][i + 0] = t3;
				B[j + 3][i + 0] = t4;
				B[j + 4][i + 0] = t5;
				B[j + 5][i + 0] = t6;
				B[j + 6][i + 0] = t7;
				B[j + 7][i + 0] = t8;
			}
		}
	}
}

/*******************************************
 case 64*64:
 The great difficulty is its mininum block
 size is 64*4, but not 32*8, so it can't
 be dealt with steps.
*******************************************/
void test64(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, k, t1, t2, t3, t4, t5, t6, t7, t8;
	for (j = 0; j < N; j += 8) {
		for (i = 0; i < M; i += 8) {
			// if i == j, it will collides on itself
			// so i store collision in variables
			// and store it to memory a little later
			if (i == j) {
				t1 = A[i + 0][j + 0];
				t5 = A[i + 0][j + 4];
				B[j + 1][i + 0] = A[i + 0][j + 1];
				B[j + 2][i + 0] = A[i + 0][j + 2];
				B[j + 3][i + 0] = A[i + 0][j + 3];
				B[j + 5][i + 0] = A[i + 0][j + 5];
				B[j + 6][i + 0] = A[i + 0][j + 6];
				B[j + 7][i + 0] = A[i + 0][j + 7];

				t2 = A[i + 1][j + 1];
				t6 = A[i + 1][j + 5];
				B[j + 4][i + 0] = t5;
				B[j + 4][i + 1] = A[i + 1][j + 4];
				B[j + 6][i + 1] = A[i + 1][j + 6];
				B[j + 7][i + 1] = A[i + 1][j + 7];
				B[j + 0][i + 1] = A[i + 1][j + 0];
				B[j + 0][i + 0] = t1;
				B[j + 2][i + 1] = A[i + 1][j + 2];
				B[j + 3][i + 1] = A[i + 1][j + 3];
				
				t3 = A[i + 2][j + 2];
				t7 = A[i + 2][j + 6];
				B[j + 0][i + 2] = A[i + 2][j + 0];
				B[j + 1][i + 1] = t2;
				B[j + 1][i + 2] = A[i + 2][j + 1];
				B[j + 3][i + 2] = A[i + 2][j + 3];
				B[j + 4][i + 2] = A[i + 2][j + 4];
				B[j + 5][i + 1] = t6;
				B[j + 5][i + 2] = A[i + 2][j + 5];
				B[j + 7][i + 2] = A[i + 2][j + 7];
				
				t4 = A[i + 3][j + 3];
				t8 = A[i + 3][j + 7];
				B[j + 4][i + 3] = A[i + 3][j + 4];
				B[j + 5][i + 3] = A[i + 3][j + 5];
				B[j + 6][i + 3] = A[i + 3][j + 6];
				B[j + 6][i + 2] = t7;
				B[j + 0][i + 3] = A[i + 3][j + 0];
				B[j + 1][i + 3] = A[i + 3][j + 1];
				B[j + 2][i + 2] = t3;
				B[j + 2][i + 3] = A[i + 3][j + 2];

				t1 = A[i + 4][j + 4];
				t5 = A[i + 4][j + 0];
				B[j + 1][i + 4] = A[i + 4][j + 1];
				B[j + 2][i + 4] = A[i + 4][j + 2];
				B[j + 3][i + 4] = A[i + 4][j + 3];
				B[j + 3][i + 3] = t4;
				B[j + 5][i + 4] = A[i + 4][j + 5];
				B[j + 6][i + 4] = A[i + 4][j + 6];
				B[j + 7][i + 4] = A[i + 4][j + 7];
				B[j + 7][i + 3] = t8;

				t2 = A[i + 5][j + 5];
				t6 = A[i + 5][j + 1];
				B[j + 4][i + 5] = A[i + 5][j + 4];
				B[j + 4][i + 4] = t1;
				B[j + 6][i + 5] = A[i + 5][j + 6];
				B[j + 7][i + 5] = A[i + 5][j + 7];
				B[j + 0][i + 5] = A[i + 5][j + 0];
				B[j + 0][i + 4] = t5;
				B[j + 2][i + 5] = A[i + 5][j + 2];
				B[j + 3][i + 5] = A[i + 5][j + 3];

				t3 = A[i + 6][j + 6];
				t7 = A[i + 6][j + 2];
				B[j + 0][i + 6] = A[i + 6][j + 0];
				B[j + 1][i + 6] = A[i + 6][j + 1];
				B[j + 1][i + 5] = t6;
				B[j + 3][i + 6] = A[i + 6][j + 3];
				B[j + 4][i + 6] = A[i + 6][j + 4];
				B[j + 5][i + 6] = A[i + 6][j + 5];
				B[j + 5][i + 5] = t2;
				B[j + 7][i + 6] = A[i + 6][j + 7];
				
				t4 = A[i + 7][j + 7];
				t8 = A[i + 7][j + 3];
				B[j + 4][i + 7] = A[i + 7][j + 4];
				B[j + 5][i + 7] = A[i + 7][j + 5];
				B[j + 6][i + 7] = A[i + 7][j + 6];
				B[j + 6][i + 6] = t3;
				B[j + 0][i + 7] = A[i + 7][j + 0];
				B[j + 1][i + 7] = A[i + 7][j + 1];
				B[j + 2][i + 7] = A[i + 7][j + 2];
				B[j + 2][i + 6] = t7;
				B[j + 7][i + 7] = t4;
				B[j + 3][i + 7] = t8;
			}
			// at first i tried to move data directly
			// and that costs 18 misses per round
			// then i know a method that uses B[]
			// as data buffer and can 
			// reduce misses to 16/round
			if (i != j) {
				/*t1 = A[i + 0][j + 4];
				t2 = A[i + 0][j + 5];
				t3 = A[i + 0][j + 6];
				t4 = A[i + 0][j + 7];
				t5 = A[i + 1][j + 4];
				t6 = A[i + 1][j + 5];
				t7 = A[i + 1][j + 6];
				t8 = A[i + 1][j + 7];
				for (t9 = i; t9 < i + 8; t9++) 
					for (t10 = j; t10 < j + 4; t10++) 
						B[t10][t9] = A[t9][t10];
				for (t9 = i + 7; t9 > i + 1; t9--)
					for (t10 = j + 4; t10 < j + 8; t10++)
						B[t10][t9] = A[t9][t10];
				B[j + 4][i + 0] = t1;
				B[j + 5][i + 0] = t2;
				B[j + 6][i + 0] = t3;
				B[j + 7][i + 0] = t4;
				B[j + 4][i + 1] = t5;
				B[j + 5][i + 1] = t6;
				B[j + 6][i + 1] = t7;
				B[j + 7][i + 1] = t8;*/
				for (k = i; k < i + 4; k++) {
					t1 = A[k + 0][j + 0];
					t2 = A[k + 0][j + 1];
					t3 = A[k + 0][j + 2];
					t4 = A[k + 0][j + 3];
					t5 = A[k + 0][j + 4];
					t6 = A[k + 0][j + 5];
					t7 = A[k + 0][j + 6];
					t8 = A[k + 0][j + 7];

					B[j + 0][k + 0] = t1;
					B[j + 1][k + 0] = t2;
					B[j + 2][k + 0] = t3;
					B[j + 3][k + 0] = t4;
					// buffer next for line
					B[j + 0][k + 4] = t5;
					B[j + 1][k + 4] = t6;
					B[j + 2][k + 4] = t7;
					B[j + 3][k + 4] = t8;
				}
				for (k = j; k < j + 4; k++) {
					t1 = B[k + 0][i + 4];
					t2 = B[k + 0][i + 5];
					t3 = B[k + 0][i + 6];
					t4 = B[k + 0][i + 7];
					t5 = A[i + 4][k + 0];
					t6 = A[i + 5][k + 0];
					t7 = A[i + 6][k + 0];
					t8 = A[i + 7][k + 0];

					B[k + 0][i + 4] = t5;
					B[k + 0][i + 5] = t6;
					B[k + 0][i + 6] = t7;
					B[k + 0][i + 7] = t8;
					B[k + 4][i + 0] = t1;
					B[k + 4][i + 1] = t2;
					B[k + 4][i + 2] = t3;
					B[k + 4][i + 3] = t4;
					
				}
				for (k = j + 4; k < j + 8; k++) {
					t5 = A[i + 4][k + 0];
					t6 = A[i + 5][k + 0];
					t7 = A[i + 6][k + 0];
					t8 = A[i + 7][k + 0];

					B[k + 0][i + 4] = t5;
					B[k + 0][i + 5] = t6;
					B[k + 0][i + 6] = t7;
					B[k + 0][i + 7] = t8;
				}
			}
		}
	}
}

/*******************************************
 case 61*67:
 This is a tricky solution, which doesn't
 used any steps or buffers. It copys data
 directly, and with the "tested" step it
 will not always collides on itself!
*******************************************/
void test61(int M, int N, int A[N][M], int B[M][N])
{
	int c, r, t1, t2;
	if (M == 61 && N == 67) {
		for (c = 0; c < M; c += 17) {
			for (r = 0; r < N; r += 17) {
				for (t1 = r;
				t1 < r + 17 && t1 < N; t1++) {
					for (t2 = c;
					t2 < c + 17 && t2 < M; t2++) {
						B[t2][t1] = A[t1][t2];
					}
				}
			}
		}
	}
}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	if (M == 32) test32(M, N, A, B);
	if (M == 64) test64(M, N, A, B);
	if (M == 61) test61(M, N, A, B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}


/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

