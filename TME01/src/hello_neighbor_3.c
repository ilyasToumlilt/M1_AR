/**
 * TP01 - Exo4 - Hello Neighbor
 * Question 3
 *
 * @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
 * @copyright (c) 2015, toumlilt
 *
 * @version 1.0
 * @package toumlilt/M1/AR
 */

#include <stdio.h>
#include <string.h>
#include <mpi.h>

#define MSG_SIZE 64

int main(int argc, char** argv)
{
  int rank;
  int size;
  int tag = 0;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  char msg[MSG_SIZE];
  sprintf(msg, "Wassup neighbor ! - From %d", rank);

  /* le process 0 ne fera pas comme les autres */
  if( rank ){
    MPI_Ssend(msg, strlen(msg)+1, MPI_CHAR, (rank + 1) % size, tag, MPI_COMM_WORLD);
    
    MPI_Recv(msg, MSG_SIZE, MPI_CHAR, (rank + ( size - 1)) % size, tag, MPI_COMM_WORLD, &status);
  } else {
    MPI_Recv(msg, MSG_SIZE, MPI_CHAR, (rank + ( size - 1)) % size, tag, MPI_COMM_WORLD, &status);
    MPI_Ssend(msg, strlen(msg)+1, MPI_CHAR, (rank + 1) % size, tag, MPI_COMM_WORLD);
  }
  printf("%d received : %s\n", rank, msg);
  
  MPI_Finalize();
  
  return 0;
}
