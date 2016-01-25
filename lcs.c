
//Program: Parallel Least Common Subsequence
//Author:  Andrew Lewis
//Date:    November 12, 2015
//Course:  CMSC 441

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>


//this function opens the file, imports len number of characters from the file
//and inserts them into a string, which is returned
//will fail if file is invalid, empty or the string is too large
char * get_string(char *file, int len) {
  FILE *inFile = fopen(file, "r");

  if (inFile == NULL) {
    fprintf(stderr, "File read error!\n");
    exit(-1);
  }

  char *str = (char *)malloc(len+1);
  memset(str, '-', len);
  if (str == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(-1);
  }

  if(fread(str, sizeof(char), len, inFile) == 0) {
    fprintf(stderr, "Error writing to string.\n");
    exit(-1);
  }

  fclose(inFile);
  str[len] = '\0';
  return str;
}


//used in lcs_length function, returns the min of two integers
int min(int x, int y) {
  return x < y ? x : y;
}



void lcs_length(char *x, char *y, int** arr) { 
  //i,j are variables used to specify array indices in the nested for loop below
  int i, j;
  //add 1 to account for the fact that the array has extra row and col
  int num_row_entries = strlen(x) + 1;
  int num_col_entries = strlen(y) + 1;

  //there are num_col_entries + num_row_entries diagonals through the board
  for (int diag_counter = 1;
       diag_counter < num_row_entries + num_col_entries;
       ++diag_counter) {
    //sets up the proper values for iterating through the diagonal entries
    if (diag_counter > num_row_entries) {
      //if diag_counter > max row index, set i(row index) to num_row_entries
      i = num_row_entries;
      //increase starting point for col index(j) in inner loop 
      j = diag_counter - num_row_entries;
    }
    else {
      //no change to col index, row index starts at count 
      i = diag_counter;
      j = 0;
    }
    //determines the number of entries in the diagonal to be processed
    int diag_length = min(i, num_col_entries - j);


#pragma omp parallel for num_threads(omp_get_max_threads())
    //modification of book's algorithm to compute the length of
    //each subsequence in the diagonal in parallel
    for (int k = 1; k < diag_length; ++k) {
      int di = i - k;
      int dj = j + k;
      if (x[di] == y[dj]) {
	arr[di][dj] = arr[di - 1][dj - 1] + 1;
      }
      else if (arr[di - 1][dj] >= arr[di][dj - 1])
	arr[di][dj] = arr[di - 1][dj];
      else
	arr[di][dj] = arr[di][dj - 1];	 
    }
  }
} 

  
char* lcs_reconstruct(int **arr, char *x, char *y)  {
  int row = strlen(x);
  int col = strlen(y);
  //number of elements in longest subsequence
  int len = arr[row][col];
  char *lcs = (char *)malloc(len);
  //initialize values in array
  memset(lcs, '-', len);
  //algorithm to reconstruct the lcs, uses the method from hw6
  while(row > 0 && col > 0) {
    if (x[row] == y[col]) {
      lcs[len] = x[row];
      --len;
      --row;
      --col;
    }
    else if (arr[row - 1][col] >= arr[row][col - 1]) {
      --row;
    }
    else {
      --col;
    }
  }
    if(x[row] == y[col]) {
     lcs[len] = x[row];
   }
  return lcs;
}


int main(int argc, char *argv[]) {
  char *str1;
  char *str2;
  int i;
  
  if (argc != 5) {
    fprintf(stdout, "Bad format! %s file1 len1 file2 len2\n", argv[0]);
    exit(-1);
  }

  //retrieves the two strings of specified lengths from files
  str1 = get_string(argv[1], atoi(argv[2]));
  str2 = get_string(argv[3], atoi(argv[4]));
  
  //add 1 for convenience in making 2d array below
  int strlen1 = strlen(str1) + 1;
  int strlen2 = strlen(str2) + 1;
  
  //2d array used to store values of lcs for each substring
  int **array = (int**)calloc(strlen1, sizeof(int *));
  for (i = 0; i < strlen1; ++i) {
    array[i] = (int*)calloc(strlen2, sizeof(int));
  }

  //used to determine length of duration in lcs length
  double t0 = omp_get_wtime();

  lcs_length(str1, str2, array);
  
  //length of time is difference from current time from initial
   double t = omp_get_wtime() - t0;
   printf("Total time in LCS-LENGTH: %f\n", t);
   
  //array is reconstructed and printed here
  char * lcs = lcs_reconstruct(array, str1, str2);
  printf("Longest subsequence has length %d. An lcs is %s\n", strlen(lcs), lcs);

}
