/**
 * TP04 - EXO_01 - l'algorithme Phase
 *
 * - Les fonctions main et simulateur sont fournis avec le tme,
 *   j'ai modifié le simulateur pour qu'il choisisse aléatoirement
 *   un initiateur, et pour qu'il attende les décisions.
 * - Les décisions sont simplement envoyées au process 0 qui les affiche.
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
#include <limits.h>

#define TAGINIT 0
#define NB_SITE 6

#define DIAMETRE 4		/* !!!!! valeur a initialiser !!!!! */

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
  int nb_proc,rang;
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

void simulateur(void) {
  int i;

  /* nb_voisins_in[i] est le nombre de voisins entrants du site i */
  /* nb_voisins_out[i] est le nombre de voisins sortants du site i */
  int nb_voisins_in[NB_SITE+1] = {-1, 2, 1, 1, 2, 1, 1};
  int nb_voisins_out[NB_SITE+1] = {-1, 2, 1, 1, 1, 2, 1};

  int min_local[NB_SITE+1] = {-1, 4, 7, 1, 6, 2, 9};

  /* liste des voisins entrants */
  int voisins_in[NB_SITE+1][2] = {{-1, -1},
				  {4, 5}, {1, -1}, {1, -1},
				  {3, 5}, {6, -1}, {2, -1}};
                               
  /* liste des voisins sortants */
  int voisins_out[NB_SITE+1][2] = {{-1, -1},
				   {2, 3}, {6, -1}, {4, -1},
				   {1, -1}, {1, 4}, {5,-1}};

  for(i=1; i<=NB_SITE; i++){
    MPI_Send(&nb_voisins_in[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
    MPI_Send(&nb_voisins_out[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
    MPI_Send(voisins_in[i], nb_voisins_in[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
    MPI_Send(voisins_out[i], nb_voisins_out[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);    
    MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
  }

  /* choix aléatoire de l'initiateur */
  int buf = INT_MAX;
  srand(getpid());
  int p_elu = ((rand()%NB_SITE)+1);
  MPI_Send(&buf, 1, MPI_INT, p_elu, TAGINIT, MPI_COMM_WORLD);
  printf("Process 0 a choisi %d comme initiateur\n", p_elu);

  /* attente des décisions */
  MPI_Status status;
  for(i=0; i<NB_SITE; i++){
    MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    printf("Process %d returned min=%d\n", status.MPI_SOURCE, buf);
  }
}

void calcul_min(int rang)
{
  /******* partie initialisation *******/
  
  /* reception du nb_voisins_in */
  int nb_voisins_in;
  MPI_Status status;
  MPI_Recv(&nb_voisins_in, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

  /* reception du nb_voisins_out */
  int nb_voisins_out;
  MPI_Recv(&nb_voisins_out, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

  /* reception de la liste des voisins in */
  int voisins_in[nb_voisins_in];
  MPI_Recv(&voisins_in, nb_voisins_in, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

  /* reception de la liste des voisins out */
  int voisins_out[nb_voisins_out];
  MPI_Recv(&voisins_out, nb_voisins_out, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

  /* reception du minimum local */
  int min_local;
  MPI_Recv(&min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD, &status);

  /******* Partie Calcul *******/
  
  int buf;       /* les communications se feront par int */
  int phase = 0; /* phase counter */
  int i;

  /* init tableau des compteurs de réceptions */
  int received[nb_voisins_in];
  for(i=0; i<nb_voisins_in; i++)
    received[i]=0;
  
  /* communication loop */
  while(1){

    /* attente de réception */
    MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
    min_local = min(min_local, buf);
    for(i=0; i<nb_voisins_in; i++){
      if( voisins_in[i] == status.MPI_SOURCE ){
	received[i]++;
	break;
      }
    }
    
    /** send ? **/
    i=0;
    /* chaque proc envoie DIAMETRE fois */
    if( phase < DIAMETRE ){ 
      /* Un proc n'a le droit d'émettre un message à tous ses voisins 
	 sortants pour la (phase+1)ème fois qu'après avoir recu le
	 (phase)ème message de tous ses voisins entrants */
      for(; i<nb_voisins_in; i++){ 
	if( received[i] < phase )
	  break;
      }
    }
    /* je send si tout est bon */
    if( i == nb_voisins_in ){
      for(i=0; i<nb_voisins_out; i++){
	MPI_Send(&min_local, 1, MPI_INT, voisins_out[i],TAGINIT,MPI_COMM_WORLD);
      }
      phase++;
    }

    /* décision ? */
    i=0;
    /* un proc décide lorsqu'il a recu DIAMETRE messages de tous voisins_in */
    if( phase >= DIAMETRE ){
      for(; i<nb_voisins_in; i++){
	if( received[i] < DIAMETRE )
	  break;
      }
    }
    /* si tout est bon je quitte la boucle pour décider */
    if( i == nb_voisins_in )
      break;
  }

  /* tout le monde décide */
  MPI_Send(&min_local, 1, MPI_INT, 0, TAGINIT, MPI_COMM_WORLD);
  
}
