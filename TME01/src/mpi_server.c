/**
 * TP01 - Exo5 - Serveur MPI
 * Implementation
 *
 * @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
 * @copyright (c) 2015, toumlilt
 *
 * @version 1.0
 * @package toumlilt/M1/AR
 */

#include <mpi_server.h>

static server the_server;

int end_flag = 0;

void* mainServer(void* args)
{
  MPI_Status status;
  int flag; // Flag de présence d'un message
  
  /* boucle principale */
  while(1){
    /* début zone critique */
    if( pthread_mutex_lock(&(the_server.mutex)) == EINVAL ){
      perror("Error: Mutex non initialisé\n");
      exit(1);
    }

    /* test du buffer de réception */
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, status);
    
    /* relâcher le verrou */
    pthread_mutex_unlock(&(the_server.mutex));

    /* si message il y a */
    if( flag ){
      the_server.callback(status.MPI_TAG, status.MPI_SOURCE);
    }

    /* s'il faut se terminer */
    if( end_flag ){
      pthread_exit(NULL);
    }
  }
}

/* initialiser le serveur */
void start_server(void (*callback) (int rag, int source))
{
  the_server.mutex = PTHREAD_MUTEX_INITIALIZER;
  the_server.callback = callback;
  pthread_create(&(the_server.listener), NULL, mainServer, NULL);
}

/* detruire le serveur */
void destroy_server()
{
  end_flag = 1;
  pthread_join(the_server.listener, NULL);
  pthread_mutex_destroy(&(the_server.mutex));
}

/* renvoyer une référence sur le mutex */
pthread_mutex_t* getMutex(){
  return &(the_server.mutex);
}
