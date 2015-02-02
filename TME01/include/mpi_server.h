/**
 * TP01 - Exo5 - Serveur MPI
 * Code fournis - header
 *
 * @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
 * @copyright (c) 2015, toumlilt
 *
 * @version 1.0
 * @package toumlilt/M1/AR
 */

#ifndef MPI_SERVER
#define MPI_SERVER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <mpi.h>

typedef struct server{
  pthread_t listener;
  pthread_mutex_t mutex;
  void (*callback) (int tag, int source); // procédure de réception de message
} server;

/* initialiser le serveur */
void start_server(void (*callback) (int tag, int source));

/* detruire le serveur */
void destroy_server();

/* renvoyer une référence sur le mutex */
pthread_mutex_t* getMutex();

#endif
