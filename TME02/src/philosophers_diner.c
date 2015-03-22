/**
 * TP02 - EXO_01 - Diner des philosophes
 *
 * @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
 * @copyright (c) 2015, toumlilt
 *
 * @version 1.1
 * @package toumlilt/M1/AR
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi.h>

/* états des philosophes : */
#define THINKING 0 /* penser pendant un temps indéterminé */
#define HUNGRY   1 /* être affamé ( temps déterminé ) */
#define EATING   2 /* manger pendant un temps déterminé et fini */

/* messages relatifs aux demandes de baguettes */
#define WANNA_CHOPSTIK 10 /* donnes moi la baguette */
#define CHOPSTIK_YOURS 11 /* tiens la voilà */
#define DONE_EATING    12 /* j'ai fini de manger, je veux me casser */

#define NB_MEALS 3 /* nombre de repas qu'un philosophe souhaite manger */

/*******************************************************************************
 * Private Declarations
 ******************************************************************************/
void init_philosopher();
void iThink();
void wannaEat();
void iEat();
void wannaLeave();
void iLeave();
void updateState();
void* messageHandler(void* args);

/*******************************************************************************
 * Local Implementation
 ******************************************************************************/

struct _philosopher {
  int id;         /* identifiant unique par philosophe */
  int nb_philos;  /* un philo a connaissance du nb de philos présents */
  int state;      /* état du philo */
} p;

/* flags de possession des baguettes : */
int left_flag;  /* 1: je possède la baguette gauche, 0: sinon */ 
int right_flag; /* 2: je possède la baguette droite, 0: sinon */

/* parfois, quand j'ai fini de manger, je dois faire tourner la baguette */
int at_left;  /* 1 ou 0, selon si le voisin gauche attend ou pas la baguette */
int at_right; /* 1 ou 0, selon si le voisin droit attend la baguette ou pas */

/* thread de traitement des messages reçus et variables de synchro */
pthread_t msg_handler;
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  state_cond  = PTHREAD_COND_INITIALIZER;

int left_neighbor;  /* id du voisins gauche */
int right_neighbor; /* id du voisins droit  */

int finished_counter; /* nombre de process qu'ont fini de manger */

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  /* init */
  init_philosopher();

  /* main loop */
  int i;
  for(i=0; i<NB_MEALS; i++){
    iThink();
    wannaEat();
    iEat();
  }
  wannaLeave();
  iLeave();

  MPI_Finalize();
  return 0;
}

void init_philosopher()
{
  /* récupération du nb de process total : nb des philosophes */
  MPI_Comm_size(MPI_COMM_WORLD, &(p.nb_philos));
  
  /* récupération de l'id du process courant */
  MPI_Comm_rank(MPI_COMM_WORLD, &(p.id));
  
  /* initialement le philosophe va penser */
  p.state = THINKING;

  /* initialement je possède la baguette dont je suis prioritaire */
  left_flag = ( p.id ) ? 1 : 0;
  right_flag = ( p.id == p.nb_philos - 1 ) ? 1 : 0;

  /* initialement je ne suis pas encore dérangé */
  at_left = at_right = 0;

  /* init du thread traitement des messages reçus */
  pthread_create(&msg_handler, NULL, messageHandler, NULL);

  /* ids des voisins ( pour pas les retaper à chaque fois ) */
  left_neighbor  = ( p.id - 1 + p.nb_philos ) % p.nb_philos;
  right_neighbor = ( p.id + 1 ) % p.nb_philos;

  /* initialement mes voisins n'ont pas encore fini */
  finished_counter = 0;

  #ifdef VERBOSE
  printf("DEBUG: PHILO_%d initialized, neighbors: (%d,%d)\n", p.id,
	 left_neighbor, right_neighbor);
  #endif
}

void iThink()
{
  int max_duration = 5; /* durée max en secondes d'une reflexion */
  srand(p.id);
  int duration = (rand() % max_duration) + 1;

  #ifdef VERBOSE
  printf("DEBUG: Process %d will think for %d sec\n", p.id, duration);
  #endif
  
  sleep(duration);
  
  /* après avoir pensé, le philo aura systématiquement faim */
  updateState(HUNGRY);
}

void wannaEat()
{
  /* les messages seront sous forme d'entiers */
  int buf = 0;
  
  /* demande asynchrone des deux baguettes */
  pthread_mutex_lock(&state_mutex);
  if( !left_flag )
    MPI_Send(&buf, 1, MPI_INT, left_neighbor, WANNA_CHOPSTIK, MPI_COMM_WORLD);
  if( !right_flag )
    MPI_Send(&buf, 1, MPI_INT, right_neighbor, WANNA_CHOPSTIK, MPI_COMM_WORLD);

  /* attente d'accord */
  while( !left_flag || !right_flag ){
    pthread_cond_wait(&state_cond, &state_mutex);
  }

  /* yes je peux manger */
  p.state = EATING;

  pthread_mutex_unlock(&state_mutex);
}

void iEat()
{
  #ifdef VERBOSE
  printf("DEBUG: Process %d will take 2secs to eat\n", p.id);
  #endif

  sleep(2);

  /* Je suis gentil je rends les baguettes ( s'ils ont en besoin ) */
  pthread_mutex_lock(&state_mutex);
  int buf = 0; /* je ne veux pas qu'ils me les retournent */
  if( at_left ){
    MPI_Send(&buf, 1, MPI_INT, left_neighbor, CHOPSTIK_YOURS, MPI_COMM_WORLD);
    at_left = 0;
    left_flag = 0;
  }
  if( at_right ){
    MPI_Send(&buf, 1, MPI_INT, right_neighbor, CHOPSTIK_YOURS, MPI_COMM_WORLD);
    at_right = 0;
    right_flag = 0;
  }
  pthread_mutex_unlock(&state_mutex);
  
  updateState(THINKING);
}

void wannaLeave()
{
  #ifdef VERBOSE
  printf("DEBUG: Process %d wanna leave !\n", p.id);
  #endif
  
  /* messages sous forme d'entiers encore */
  int buf = 0, i;
  MPI_Status status;

  /* je n'ai plus besoin des baguettes dont j'avais possession */
  pthread_mutex_lock(&state_mutex);
  if( /*!left_finished &&*/ left_flag ){
    MPI_Send(&buf, 1, MPI_INT, left_neighbor, CHOPSTIK_YOURS, MPI_COMM_WORLD);
    left_flag = 0;
  }
  if( /*!right_finished &&*/ right_flag ){
    MPI_Send(&buf, 1, MPI_INT, right_neighbor, CHOPSTIK_YOURS, MPI_COMM_WORLD);
    right_flag = 0;
  }
  
  /* le philo commence par avertir ses voisins qu'il a fini de manger 
     Remarque, le tag utilisé est différent */
  for( i=0; i<p.nb_philos; i++)
    if( i != p.id )
      MPI_Send(&buf, 1, MPI_INT, i, DONE_EATING, MPI_COMM_WORLD);
  
  /* il doit ensuite attendre gentillement que ses voisins se soient terminés */
  finished_counter++;
  while( finished_counter < p.nb_philos )
    pthread_cond_wait(&state_cond, &state_mutex);
  
  pthread_mutex_unlock(&state_mutex);
}

void iLeave()
{
  #ifdef VERBOSE
  printf("DEBUG: Process %d, i'm leaving baby !\n", p.id);
  #endif
  
  /* je range quand même mes affaires avant de partir */
  pthread_cancel(msg_handler);
  sleep(1);
}

void updateState(int newState)
{
  pthread_mutex_lock(&state_mutex);
  p.state = newState;
  if( newState != EATING )
    pthread_cond_broadcast(&state_cond);
  pthread_mutex_unlock(&state_mutex);
}

/*******************************************************************************
 * Partie relative au thread de traitement des messages recus
 ******************************************************************************/

void* messageHandler(void* args)
{
  MPI_Status status;
  int buf;

  while(1){
    MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, 
	     MPI_COMM_WORLD, &status);
    
    pthread_mutex_lock(&state_mutex);
    
    if( status.MPI_TAG == WANNA_CHOPSTIK ){
      /* non ducon */
      
      /* laisses moi d'abord finir de manger après on verra */
      while( p.state == EATING ){
	pthread_cond_wait(&state_cond, &state_mutex);
      }
      
      /* si j'ai faim et que je suis plus fort que le gars, je me sers d'abord */
      if( p.state == HUNGRY && p.id > status.MPI_SOURCE){
	if( status.MPI_SOURCE ){
	  at_left = 1;
	  left_flag = 1;
	} else { 
	  at_right = 1;
	  right_flag = 0;
	}
	pthread_cond_broadcast(&state_cond);
      } else {
	if( ( status.MPI_SOURCE == p.id + 1 ) ||
	    ( p.id == p.nb_philos - 1 && !status.MPI_SOURCE ) ){
	  right_flag = 0;
	} else {
	  left_flag = 0;
	}
	buf = ( p.state == HUNGRY ) ? 1 : 0;
	MPI_Send(&buf, 1, MPI_INT, status.MPI_SOURCE, CHOPSTIK_YOURS,
		 MPI_COMM_WORLD);
      }
    }
    else if ( status.MPI_TAG == CHOPSTIK_YOURS ) {
      if( ( status.MPI_SOURCE == p.id + 1 ) ||
	  ( p.id == p.nb_philos - 1 && !status.MPI_SOURCE ) ){
	right_flag = 1;
	at_right = buf;
      } else {
	left_flag = 1;
	at_left = buf;
      }
      pthread_cond_broadcast(&state_cond);
    }
    else { /* status.MPI_TAG == DONE_EATING */
      finished_counter++;
      pthread_cond_broadcast(&state_cond);
    }

    pthread_mutex_unlock(&state_mutex);
  }
}


