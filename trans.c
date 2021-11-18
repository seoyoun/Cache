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

void transpose_32(int M, int N, int A[N][M], int B[M][N]){
    /*
        int blocksize = 8;
        for (int row = 0; row < N; row += blocksize){
            for (int col = 0; col < M; col += blocksize){
                for (int i = row; i < row + blocksize && i < N; ++i){
                    for (int j = col; j < col + blocksize && j < M; ++j){
                        if (i != j) {
                            //int a = A[i][j];
                            B[j][i] = A[i][j];
                        } 
                    }
                    if (row == col) {
                        //int a = A[i][i];
                        B[i][i] = A[i][i];
                    }
                }
            }
        }
    */
    int blocking = 8;
    //use 8x8 blocks except for the diagonals
    for(int start_row = 0; start_row < N; start_row += blocking){
        //int row = start_row;
        for(int start_col = 0; start_col < M; start_col += blocking){
            
                for(int row = start_row; row < start_row + blocking; row++){
                
                    for(int col = start_col; col < start_col + blocking; col++){
                        
                        if(row != col){
                            B[col][row] = A[row][col];
                        }
                        
                    }
                    
                    if(start_row == start_col){
                        B[row][row] = A[row][row];
                    }
                    
                
                
                }
           
            
        }
        
    }
}

void transpose_64(int M, int N, int A[N][M], int B[M][N]){
    /*
    int blocking = 4;
   
    //use 8x8 blocks except for the diagonals
    for(int start_row = 0; start_row < N; start_row += blocking){
        
        for(int start_col = 0; start_col < M; start_col += blocking){

            
                for(int row = start_row; row < start_row + blocking; row++){
                
                    for(int col = start_col; col < start_col + blocking; col++){
                        
                        if(row != col){
                            B[col][row] = A[row][col];
                        }
                        
                    
                        
                    }
                    
                    if(start_row == start_col){
                        B[row][row] = A[row][row];
                    }
                    
                
                
                }
           
            
        }
        
    }
    */
    
    for(int start_row = 0; start_row < N; start_row += 8){
        
        for(int start_col = 0; start_col < M; start_col += 4){

            for(int row = start_row; row < start_row + 8; row++){
            
                for(int col = start_col; col < start_col + 4; col++){
                    
                    if(row != col){
                        B[col][row] = A[row][col];
                    }
                    
                }
                
                if(start_row == start_col){
                    B[row][row] = A[row][row];
                }
                
            }
           
        }
        
    }



    
}

//transpose function for 61x67 matrix
void transpose_61(int M, int N, int A[N][M], int B[M][N]){

    int blocking = 14; //will transpose 14 blocks at a time
  
    for(int start_row = 0; start_row < N; start_row += blocking){ //first row of a 14x14 block
        
        for(int start_col = 0; start_col < M; start_col += blocking){ //first column of a 14x14 block

            for(int row = start_row; row < start_row + blocking && row < N; row++){ //each row of a 14x14 block
            
                for(int col = start_col; col < start_col + blocking && col < M; col++){ //each column of a 14x14 block
            
                    B[col][row] = A[row][col];
                    
                }
                
            }
           
            
        }
        
    }
}

void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if(M == 32){
        transpose_32(M, N, A, B);
    }else if(M == 64){
        transpose_64(M, N, A, B);
    }else if(M == 61){
        transpose_61(M, N, A, B);
    }
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

