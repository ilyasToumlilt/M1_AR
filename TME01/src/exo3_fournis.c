/**
 * TP01 - Exo3 - Hello Master - fichier fournis
 *
 * @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
 * @copyright (c) 2015, toumlilt
 *
 * @version 1.0
 * @package toumlilt/M1/AR
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MASTER 0
#define SIZE 128

int main(int argc, char** argv)
{
  int my_rank;
  int nb_proc;
  int source;
  int dest;
  int tag=0;
  char message[SIZE];

  MPI_Status status;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if( my_rank != MASTER ){
    sprintf(message, "Hello Master from %d", my_rank);
    dest = MASTER;
    MPI_Send(message, strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
  } else {
    for(source=0; source<nb_proc; source++){
      if(source != my_rank){
	MPI_Recv(message, SIZE, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	printf("%s\n", message);
      }
    }
  }

  MPI_Finalize();

  return 0;
}
