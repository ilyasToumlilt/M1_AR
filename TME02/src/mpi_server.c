/* code fournis par le prof */
void* procedure_listner(void* arg)
{
  server* arg_server = (server*)arg;
  MPI_Status status;
  int flag;
  while(1){
    pthread_mutex_lock(&(arg_server->mutex));
    MPI_Iprobe(MPI_ANY_SERVER, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    pthread_mutex_unlock(&(arg_server->mutex));
    if(flag){
      arg_server->callback(status.MPI_TAG, status.MPI_SOURCE);
    }
  }
  return NULL;
}

int start_server(void(*callback)(int tag, int source))
{
  the_server callback = callback;
  int ret = pthread_mutex_init(&(the_server.mutex), NULL);
  if( ret != 0 ){
    perror("Erreur pthread_mutex_init\n");
    return 1;
  }
  ret = pthread_create(&(the_server.listner), NULL, procedure_listner, &the_server);
  if( ret != 0 ){
    perror("Erreur pthread_create\n");
    return 2;
  }
  return 0;
}

int destroy_server()
{
  int ret = pthread_cancel(the_server.listner);
  if( !ret ){
    perror("Error pthread_cancel\n");
    return 2;
  }
  pthread_mutex_destroy(&(the_server.mutex));
  return 0;
}

pthread_mutex_t* getMutex()
{
  return &(the_server.mutex);
}
