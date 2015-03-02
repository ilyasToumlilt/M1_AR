/**
 * TP05 - Algorithme de Ricart & Agrawala
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
#include <mpi.h>
#include <pthread.h>

#define NB_CS 5

#define TAGREQUEST 90
#define TAGREPLY   91
#define TAGEND     92

#define REQUESTING       0
#define NOT_REQUESTING   1
#define CRITICAL_SECTION 2

#define max(a,b) ((a < b) ? b : a)

/*******************************************************************************
 * Private Declarations
 ******************************************************************************/
void request_CS(int rank, int size);
void release_CS(int rank, int size);
void* msg_handler(void* args);

/*******************************************************************************
 * Variables locales aux process
 ******************************************************************************/
int rank, size;
int myClock = 0; int myClockOnRequest = 0;
int* RD; /* Request-Deffered array */
int ATcounter = 0;
int state = NOT_REQUESTING;

/*******************************************************************************
 * Message handling thread
 ******************************************************************************/
pthread_t th;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/*******************************************************************************
 * Process end vars
 ******************************************************************************/
int ENDcounter = 0;

/*******************************************************************************
 * Implementation
 ******************************************************************************/
int main(int argc, char** argv)
{
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* allocation des tableaux */
  RD = (int*)malloc(size*sizeof(int));

  /* init */
  int i;
  for(i=0; i<size; i++)
    RD[i] = 0;
  
  /* init du thread traitant les réceptions */
  pthread_create(&th, NULL, msg_handler, NULL);

  srand(rank);
  /* main loop, NB_CS fois demandes */
  for(i=0; i<NB_CS; i++){
    /* 7 fois sur 10, je ne fais rien */
    while( (rand()%10) < 7 ){
      myClock++;
      /*printf("Process %d passe son tour pour 1sec\n", rank);
       */sleep(1);
    }
    /* demande de CS */
    request_CS(rank, size);
    /* en SC je fais ce que j'ai à faire */
    printf("Process %d en CS, je m'y endors 2secondes\n", rank);
    sleep(2);
    printf("Process %d, je release la CS\n", rank);
    /* release CS */
    release_CS(rank, size);
  }

  /* je préviens mes amis avant de me terminer */
  for(i=0; i<size; i++)
    if(i!=rank)
      MPI_Send(&i, 1, MPI_INT, i, TAGEND, MPI_COMM_WORLD);

  /* et comme je suis gentil, j'attends qu'ils finissent */
  while(ENDcounter < size-1 )
    pthread_cond_wait(&cond, &mutex);

  printf("Process %d se termine\n", rank);

  /* on fait le ménage */
  free(RD);
  pthread_cancel(th);
  
  MPI_Finalize();


  return EXIT_SUCCESS;
}

void request_CS(int rank, int size)
{
  pthread_mutex_lock(&mutex);
  myClock++;
  myClockOnRequest = myClock;
  state = REQUESTING;

  int i;
  /* demande aux amis */
  for(i=0; i<size; i++){
    if( i!=rank ){
      MPI_Send(&myClock, 1, MPI_INT, i, TAGREQUEST, MPI_COMM_WORLD);
    }
  }
  ATcounter = size-1;

  while( state != CRITICAL_SECTION )
    pthread_cond_wait(&cond, &mutex); 

  pthread_mutex_unlock(&mutex);
}

void release_CS(int rank, int size)
{
  pthread_mutex_lock(&mutex);
  myClock++;
  state = NOT_REQUESTING;
  
  /* notification des REPLY différés */
  int i;
  /* @todo en pratique y a pas besoin d'enregistrer les clocks recues */
  for(i=0; i<size; i++){
    if( RD[i] ){
      MPI_Send(&myClock, 1, MPI_INT, i, TAGREPLY, MPI_COMM_WORLD);
      RD[i] = 0;
    }
  }

  pthread_mutex_unlock(&mutex);
}

void* msg_handler(void* args)
{
  MPI_Status status;
  int buf, i, j;
  
  while(1) {
    /* attente de réception */
    MPI_Recv(&buf, 1, MPI_INT, MPI_ANY_SOURCE, 
	     MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    switch(status.MPI_TAG){
    case TAGREQUEST:
      pthread_mutex_lock(&mutex);
      /* clock update */
      myClock = max(myClock, buf);
      myClock++;
      /* FA update, si besoin */
      if( state == NOT_REQUESTING ||
	  ( state == REQUESTING && ( buf < myClockOnRequest ||
				     ( buf == myClockOnRequest &&
				     status.MPI_SOURCE < rank ))) ){
	myClock++;
	MPI_Send(&myClock, 1, MPI_INT, status.MPI_SOURCE,
		 TAGREPLY, MPI_COMM_WORLD);
      } else {
	RD[status.MPI_SOURCE] = 1;
      }
      pthread_mutex_unlock(&mutex);
      break;
    case TAGREPLY:
      pthread_mutex_lock(&mutex);
      /* clock update */
      myClock = max(myClock, buf);
      myClock++;
      /* AT update */
      ATcounter--;
      /* si AT vide on peut rentrer en CS */
      if( !ATcounter ){
	state = CRITICAL_SECTION;
	pthread_cond_signal(&cond);
      }
      pthread_mutex_unlock(&mutex);
      break;
    case TAGEND :
      pthread_mutex_lock(&mutex);
      ENDcounter++;
      pthread_cond_signal(&cond);
      pthread_mutex_unlock(&mutex);
      break;
    default: break;
    }
  }
  return NULL;
}
