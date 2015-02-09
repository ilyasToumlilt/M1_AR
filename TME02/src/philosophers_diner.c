/**
 * TP02 - EXO_01 - Diner des philosophes
 * Ce programme necessite une augmentation de la memoire allouee aux process
 * ( commande sh/bash : ulimit )
 *
 * @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
 * @copyright (c) 2015, toumlilt
 *
 * @version 1.0
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
void* leftChopstikHandler(void* args);
void* rightChopstikHandler(void* args);
void handleMessage(int neighbor, int* flag);

/*******************************************************************************
 * Local Implementation
 ******************************************************************************/

struct _philosopher {
  int id;         /* identifiant unique par philosophe */
  int nb_philos;  /* un philo a connaissance du nb de philos présents */
  int state;      /* état du philo */
} p;

int left_flag;
pthread_t left_chopstik;

int right_flag;
pthread_t right_chopstick;

pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  state_cond  = PTHREAD_COND_INITIALIZER;

/* debug */
char m_buff[128];
char l_buff[128];
char r_buff[128];
pthread_mutex_t d_m = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);

  /* récupération du nb de process total : nb des philosophes */
  MPI_Comm_size(MPI_COMM_WORLD, &(p.nb_philos));

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

  return EXIT_SUCCESS;
}

void init_philosopher()
{
  /* récupération de l'id */
  MPI_Comm_rank(MPI_COMM_WORLD, &(p.id));
  
  /* initialement le philosophe va penser */
  p.state = THINKING;

  /* initialement je n'ai pas de baguette en main */
  left_flag = right_flag = 0;

  /* init des deux threads de gestion des baguettes */
  pthread_create(&left_chopstik, NULL, leftChopstikHandler, NULL);
  pthread_create(&right_chopstick, NULL, rightChopstikHandler, NULL);

  #ifdef VERBOSE
  printf("DEBUG: PHILO_%d initialized, neighbors: (%d,%d)\n", p.id,
	 ((p.id - 1) + p.nb_philos) % p.nb_philos,
	 (p.id + 1) % p.nb_philos);
  #endif
}

void iThink()
{
  int max_duration = 5; /* nombre de secondes max d'une reflexion */
  srand(p.id);
  int duration = (rand() % max_duration) + 1;

  sleep(duration);

  /* après avoir pensé, le philo aura systématiquement faim */
  updateState(HUNGRY);
}

void wannaEat()
{
  /* les messages seront sous forme d'entiers */
  int buf = WANNA_CHOPSTIK;
  
  /* demande asynchrone des deux baguettes */
  MPI_Send(&buf, 1, MPI_INT, ((p.id - 1) + p.nb_philos) % p.nb_philos,
	   WANNA_CHOPSTIK, MPI_COMM_WORLD);
  MPI_Send(&buf, 1, MPI_INT, (p.id + 1) % p.nb_philos,
	   WANNA_CHOPSTIK, MPI_COMM_WORLD);

  pthread_mutex_lock(&state_mutex);
  while( !right_flag || !left_flag ){
    pthread_cond_wait(&state_cond, &state_mutex);
  }
  right_flag = left_flag = 0;
  pthread_mutex_unlock(&state_mutex);

  /* yes je peux manger */
  updateState(EATING);
}

void iEat()
{
  sleep(2);

  int buf = CHOPSTIK_YOURS;
  MPI_Send(&buf, 1, MPI_INT, ((p.id - 1) + p.nb_philos) % p.nb_philos,
	   CHOPSTIK_YOURS, MPI_COMM_WORLD);
  MPI_Send(&buf, 1, MPI_INT, (p.id + 1) % p.nb_philos,
	   CHOPSTIK_YOURS, MPI_COMM_WORLD);
  
  updateState(THINKING);
}

void wannaLeave()
{
  /* messages sous forme d'entiers encore */
  int buf = DONE_EATING;
  MPI_Status status;
  
  /* le philo commence par avertir ses voisins qu'il a fini de manger 
     Remarque, le tag utilisé pour les départs est différent */
  MPI_Send(&buf, 1, MPI_INT, ((p.id - 1) + p.nb_philos) % p.nb_philos,
	   DONE_EATING, MPI_COMM_WORLD);
  MPI_Send(&buf, 1, MPI_INT, (p.id + 1) % p.nb_philos,
	   DONE_EATING, MPI_COMM_WORLD);

  /* il doit ensuite attendre que ses voisins se soient terminés */
  MPI_Recv(&buf, 1, MPI_INT, ((p.id - 1 ) + p.nb_philos) % p.nb_philos,
	   DONE_EATING, MPI_COMM_WORLD, &status);
  MPI_Recv(&buf, 1, MPI_INT, (p.id + 1) % p.nb_philos,
	   DONE_EATING, MPI_COMM_WORLD, &status);
}

void iLeave()
{
  /* je range quand même mes affaires avant de partir */
  pthread_cancel(left_chopstik);
  pthread_cancel(right_chopstick);

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

/**
 * Ce gars s'occupe de la baguette gauche
 * et ne communiquera donc qu'avec le voisin gauche
 */
void* leftChopstikHandler(void* args)
{
  int neighbor = ((p.id - 1) + p.nb_philos) % p.nb_philos;

  while(1){
    handleMessage(neighbor, &left_flag);
  }
}

/**
 * Ce gars s'occupe de la baguette droite
 * et ne communiquera donc qu'avec le voisin droite
 */
void* rightChopstikHandler(void* args)
{
  int neighbor = (p.id + 1) % p.nb_philos;

  while(1){
    handleMessage(neighbor, &right_flag);
  }
}

void handleMessage(int neighbor, int* flag)
{
  int buf = CHOPSTIK_YOURS;
  MPI_Status status;

  MPI_Recv(&buf, 1, MPI_INT, neighbor, MPI_ANY_TAG, 
	   MPI_COMM_WORLD, &status);

  pthread_mutex_lock(&state_mutex);

  if( status.MPI_TAG == WANNA_CHOPSTIK ){
    /* non ducon */
  
    /* laisse moi d'abord finir de manger après on parlera */
    while( p.state == EATING ){
      pthread_cond_wait(&state_cond, &state_mutex);
    }

    /* si j'ai faim et que je suis plus fort que le gars, je me sers d'abord */
    if( p.state == HUNGRY && p.id > neighbor ){
      *flag = 1;
      pthread_cond_broadcast(&state_cond);
      while ( p.state != THINKING ) {
	pthread_cond_wait(&state_cond, &state_mutex);
      }
    }
    *flag = 0;
    MPI_Send(&buf, 1, MPI_INT, neighbor, CHOPSTIK_YOURS,
	     MPI_COMM_WORLD);
  } else {
    /* est-ce que j'en avais vraiment besoin ? */
    if( p.state == HUNGRY ){
      *flag = 1;
      pthread_cond_broadcast(&state_cond);
    }
  }

  pthread_mutex_unlock(&state_mutex);
}


