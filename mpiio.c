/* 
 * File: mpiio.c
 * Written by: Dmitry Duplyakin
 * 12/01/12
 *
 * This code has examples of parallel reads and writes with subarrays
 *
*/

#include <stdio.h>
#include <string.h>  
#include <mpi.h>     
#include <math.h>
#include <stdlib.h>
 
// Order of the matrix in the file
const int N = 8;
 
int main(int argc, char *argv[]) 
{
   int        comm_sz;               
   int        my_rank;              

   char* infile = "input.txt";
   char* outfile = "output.txt";

   MPI_File fh;
   MPI_Datatype filetype;
 
   // Start up MPI 
   MPI_Init(NULL, NULL);
 
   // Get the number of processes 
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
 
   // Get my rank among all the processes
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

   // printf("Openning file on rank=%d\n", my_rank);
   // fflush(stdout);
      
   int q = sqrt(comm_sz);
   int gsizes[2]={N, N}; 
   int lsizes[2]={N/q, N/q}; 
   int psizes[2]={q, q};

   // Column-major order
   // int coords[2]={my_rank%psizes[0], my_rank/psizes[0]}; 
   // Row-major order
   int coords[2]={my_rank/psizes[0], my_rank%psizes[0]}; 

   int starts[2]={coords[0]*lsizes[0], coords[1]*lsizes[1]};
   char* array;
   array = (char*)malloc(lsizes[0]*lsizes[1]*sizeof(char));
   
   // Access modes: 
   // MPI_MODE_RDONLY --- read only,
   // MPI_MODE_RDWR --- reading and writing,
   // MPI_MODE_WRONLY --- write only,
   // MPI_MODE_CREATE --- create the file if it does not exist,
   // MPI_MODE_EXCL --- error if creating file that already exists,
   // MPI_MODE_DELETE_ON_CLOSE --- delete file on close,
   // MPI_MODE_UNIQUE_OPEN --- file will not be concurrently opened elsewhere,
   // MPI_MODE_SEQUENTIAL --- file will only be accessed sequentially,
   // MPI_MODE_APPEND --- set initial position of all file pointers to end of file. 
   MPI_File_open(MPI_COMM_WORLD, infile, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);  
   
   // Example of parallel reads
   
   MPI_Type_create_subarray(2, gsizes, lsizes, starts,
           MPI_ORDER_C, MPI_CHAR, &filetype); 
   MPI_Type_commit(&filetype);
   MPI_File_set_view(fh, 0, MPI_CHAR, filetype, "native", MPI_INFO_NULL);
   MPI_File_read_all(fh, array, lsizes[0]*lsizes[1], MPI_CHAR, MPI_STATUS_IGNORE);
   MPI_File_close(&fh);

   int i;
   char output[100];
   char output_copy[100];
   sprintf(output, "Chars on rank=%d: ", my_rank);
   for (i = 0; i < lsizes[0]*lsizes[1]; ++i)
   {
       strcpy(output_copy, output);
       sprintf(output, "%s%c, ", output_copy, array[i]);
   }
   printf("%s\n", output);

   // Modify the array (turn ranks into chars)
   char c = 'A' + my_rank;
   for (i = 0; i < lsizes[0]*lsizes[1]; ++i)
   {
       array[i] = c;
   }
   
   MPI_File_open(MPI_COMM_WORLD, outfile, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);  
   
   // Example of parallel writes
   
   // No need for commiting the type again
   // MPI_Type_create_subarray(2, gsizes, lsizes, starts,
   //        MPI_ORDER_C, MPI_CHAR, &filetype); 
   // MPI_Type_commit(&filetype);

   MPI_File_set_view(fh, 0, MPI_CHAR, filetype, "native", MPI_INFO_NULL);
   MPI_File_write_all(fh, array, lsizes[0]*lsizes[1], MPI_CHAR, MPI_STATUS_IGNORE);
   MPI_File_close(&fh);

   free(array);
 
   // Shut down MPI 
   MPI_Finalize();

   return 0;
}  
