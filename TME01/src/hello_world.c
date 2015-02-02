/**
 * TP01 - Exo2 - Hello World
 *
 * @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
 * @copyright (c) 2015, toumlilt
 *
 * @version 1.0
 * @package toumlilt/M1/AR
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv)
{
  
  MPI_Init(&argc, &argv);
  
  /* récupération du rang */
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* récupération du nb de process total */
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  printf("Processus %d sur %d : Hello MPI\n", rank, size);

  MPI_Finalize();

  return EXIT_SUCCESS;
}
