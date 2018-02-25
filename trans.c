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

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
//void transpose_submit(int M, int N, int A[N][M], int B[M][N])
//{
//}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";

/*
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}*/

void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
	int big_row_num; 
	int big_col_num; 
	int block_row;
	int block_col;
	int diagonal_index=0;
	int temporary_element=0;

	//first case, 32 X 32 matrix. Using block size 8.

	if(N == 32 && M == 32){
	//iterate through the the columns of the big matrix, skipping by block size each time
	for (big_col_num =0; big_col_num < M; big_col_num += 8){
		//iterate through the rows of the big matrix, skipping by block size each time
		for(big_row_num =0; big_row_num < N; big_row_num += 8){
			//start iterating through the small block rows, starting at whatever row we are currently on and 
			//going until we reach the current row + block size
			for(block_row=big_row_num; block_row < big_row_num + 8; block_row++){
				//iterate through the small block columns, starting at whatever column we are currently on and
				//going until we reach the current row + 8
				for(block_col=big_col_num; block_col < big_col_num + 8; block_col++){
					//if we are not on a diagonal element, then need to transpose 
					if(block_row != block_col){
						//transpose the element
						B[block_col][block_row] = A[block_row][block_col];
					}
					//else it is a diagonal element of the big matrix. Store this for later to reduce conflict misses
					//between the small inner matrices and the large outer matrix
					else{
						//hold the diagonal element in a temporary element
						temporary_element = A[block_row][block_col];
						//grab the index of the diagonal element 
						diagonal_index=block_col;

					}
				}
				//this means we have a diagonal element, now we transpose it 
				if (big_row_num == big_col_num){
					B [diagonal_index][diagonal_index] = temporary_element;

				}
			}
		}

	}

}
	//case for the 64 * 64 matrix, using block size 4 as it had the bst results
	else if(N == 64 && M == 64){
		//iterate through the the columns of the big matrix, skipping by block size each time
	for (big_col_num =0; big_col_num < M; big_col_num += 4){
		//iterate through the rows of the big matrix, skipping by block size each time
		for(big_row_num =0; big_row_num < N; big_row_num += 4){
			//start iterating through the small block rows, starting at whatever row we are currently on and 
			//going until we reach the current row + block size
			for(block_row=big_row_num; block_row < big_row_num + 4; block_row++){
				//iterate through the small block columns, starting at whatever column we are currently on and
				//going until we reach the current row + 8
				for(block_col=big_col_num; block_col < big_col_num + 4; block_col++){
					//if we are not on a diagonal element, then need to transpose 
					if(block_row != block_col){
						//transpose the element
						B[block_col][block_row] = A[block_row][block_col];
					}
					//else it is a diagonal element. Store this for later to reduce conflict misses
					else{
						temporary_element = A[block_row][block_col];
						//grab the index of the diagonal element 
						diagonal_index=block_col;

					}
				}
				//this means we have a diagonal element,
				if (big_row_num == big_col_num){
					B [diagonal_index][diagonal_index] = temporary_element;
				}
			}
		}

	}
	

	}
	//else we are working with the 61*67 matrix. Using block size 16 as it got the best results
	else {
		//iterate through the the columns of the big matrix, skipping by block size each time
	for (big_col_num =0; big_col_num < M; big_col_num += 16){
		//iterate through the rows of the big matrix, skipping by block size each time
		for(big_row_num =0; big_row_num < N; big_row_num += 16){
			//start iterating through the small block rows, starting at whatever row we are currently on and 
			//going until we reach the current row + block size
			//we also need to make sure that we don't seg fault by going out of bounds on the rows 
			//this is what the (block_row < N) condition is for
			for(block_row=big_row_num; block_row < (big_row_num + 16) && (block_row < N) ; block_row++){
				//iterate through the small block columns, starting at whatever column we are currently on and
				//going until we reach the current row + block size
				//we also need to make sure that we don't seg fault by going out of bounds on the columns 
				//this is what the (block_col < M) condition is for
				for(block_col=big_col_num; (block_col < big_col_num + 16) && (block_col < M); block_col++){
					//if we are not on a diagonal element, then need to transpose 
					if(block_row != block_col){
						//transpose the element
						B[block_col][block_row] = A[block_row][block_col];
					}
					//else it is a diagonal element. Store this for later to reduce conflict misses
					else{
						temporary_element = A[block_row][block_col];
						//grab the index of the diagonal element 
						diagonal_index=block_col;

					}
				}
				//this means we have a diagonal element,
				if (big_row_num == big_col_num){
					B [diagonal_index][diagonal_index] = temporary_element;
				}
			}
		}

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
    //registerTransFunction(good_transpose, trans_desc); 

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

