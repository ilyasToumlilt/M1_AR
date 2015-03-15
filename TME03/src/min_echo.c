/**
 * TP03 - EXO_02 - l'algorithme de l'echo pour calcul de min
 * Les fonctions main et simulateur sont fournies avec l'énoncé
 * j'ai modifié le simulateur pour choisir aléatoirement un initiateur
 * et pour traiter la décision ( print )
 *
 * @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
 * @copyright (c) 2015, toumlilt
 *
 * @version 1.3
 * @package toumlilt/M1/AR
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <limits.h>

#define NB_SITE 6
#define TAGINIT 0

#define min(a,b) ((a > b) ? b : a)

/*******************************************************************************
 * Private Declarations
 ******************************************************************************/
void simulateur(void);
void calcul_min(int rang);

/*******************************************************************************
 * Local Implementation
 ******************************************************************************/
int main (int argc, char* argv[])
{
  int nb_proc, rang;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

  if (nb_proc != NB_SITE+1) {
    printf("Nombre de processus incorrect !\n");
    MPI_Finalize();
    exit(2);
  }
  
  MPI_Comm_rank(MPI_COMM_WORLD, &rang);
  
  if (rang == 0) {
    simulateur();
  } else {
    calcul_min(rang);
  }
  
  MPI_Finalize();
  return 0;
}

void simulateur(void)
{
  int i;

  /* nb_voisins[i] est le nombre de voisins du site i */
  int nb_voisins[NB_SITE+1] = {-1, 3, 3, 2, 3, 5, 2};
  int min_local[NB_SITE+1] = {-1, 12, 11, 8, 14, 5, 17};

  /* liste des voisins */
  int voisins[NB_SITE+1][5] = {{-1, -1, -1, -1, -1},
			       {2, 5, 3, -1, -1}, {4, 1, 5, -1, -1}, 
			       {1, 5, -1, -1, -1}, {6, 2, 5, -1, -1},
			       {1, 2, 6, 4, 3}, {4, 5, -1, -1, -1}};
                               
  for(i=1; i<=NB_SITE; i++){
    MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
    MPI_Send(voisins[i], nb_voisins[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
    MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD); 
  }

  /* l'initiateur sera choisi aléatoirement : */
  i = (rand() % NB_SITE) + 1;
  int max_int = INT_MAX;
  MPI_Send(&max_int, 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);

  /* attente et affichage du résultat final */
  MPI_Status status;
  MPI_Recv(&i, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
  printf("min = %d\n", i);
  
}

void calcul_min(int rang)
{
  /*** partie initialisation ***/

  /* reception du nombre de voisins */
  int nb_voisins;
  MPI_Status status;
  MPI_Recv(&nb_voisins, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

  /* reception des rangs des voisins */
  int voisins[nb_voisins];
  MPI_Recv(voisins, nb_voisins, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

  /* reception du min_local */
  int min_local;
  MPI_Recv(&min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

  /*** Partie calcul ***/
  
  int buf; /* communications par int */
  int dad = -1; /* rang du père */
  int i; 

  /* attente du message du père */
  MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
  dad = status.MPI_SOURCE;
  min_local = min(min_local, buf);

  /* echo aux autre voisins */
  for(i=0; i<nb_voisins; i++){
    if( voisins[i] != dad ){
      MPI_Send(&min_local, 1, MPI_INT, voisins[i], TAGINIT, MPI_COMM_WORLD);
    }
  }

  /* attente des retours */
  for(i=0; i<nb_voisins-1; i++){
    MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
    min_local = min(min_local, buf);
  }

  /* réponse au père */
  MPI_Send(&min_local, 1, MPI_INT, dad, TAGINIT, MPI_COMM_WORLD);
}

