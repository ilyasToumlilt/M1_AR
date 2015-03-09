#define N      3    /* Number of processes */ 
#define L     10    /* Buffer size */
#define NIL (N+1)   /* for an undefined value */

#define dem_SC (anneau[1]@ask_SC))
#define en_SC (anneau[1]@in_SC))

mtype = {req, tk};

/* le processus i recoit ses messages dans canal[i] */ 
chan canal[N] = [L] of {mtype, byte};    /* pour un message de type tk, 
                                            on mettra la valeur associee a 0 */

byte SharedVar = 0;                      /* la variable partagee
					    Je considere que cette variable
                                            représente le nombre de process
					    en SC */ 

/* Initialise les variables locales, sauf reqId, val et typ_mes.
 * proc 0 possede le jeton au depart. */
inline Initialisation ( ) {
  father = 0;
  next = NIL;
  requesting = false;
  if
    :: (father == id) ->
       token = 1;
       father = NIL;
    :: else ->
       token = 0;
  fi;
}

/* operations effectuees lors d'une demande de SC */
inline Request_CS ( ) {
  requesting = 1;
  if
    :: (father != NIL) ->
       atomic {
	 canal[father]!req,id;
	 father = NIL;
       }
    :: else -> skip;
  fi;
  (token == 1);
}

/* operations effectuees en sortie de SC */
inline Release_CS ( ) {
  requesting = 0;
  if
    :: (next != NIL) ->
       atomic {
	 canal[next]!tk,id;
	 token = false;
	 next = NIL;
       }
    :: else -> skip;
  fi;
}

/* traitement de la reception d'une requete */ 
inline Receive_Request_CS () {
  if
    :: (father == NIL) ->
       if
	 :: (requesting == 1) ->
	    next = val;
	 :: else ->
	    token = 0;
	    canal[val]!tk,id;
       fi;
    :: else ->
       canal[father]!req,val;
  fi;
  father = val;
}

/* traitement de la reception du jeton */
inline Receive_Token () {
  token = true;
}


proctype node(byte id; byte Initial_Token_Holder){
    
  bit requesting;    /* indique si le processus a demande la SC ou pas  */
  bit token;         /* indique si le processus possede le jeton ou non */
  
  byte father;       /* probable owner */
  byte next;         /* next node to whom send the token */
  byte val;          /* la valeur contenue dans le message */
  mtype typ_mes;     /* le type du message recu */
  byte reqId;        /* l'Id du demandeur, pour une requete */

  chan canal_rec = canal[id];     /* un alias pour mon canal de reception */
  xr canal_rec;                   /* je dois etre le seul a le lire */

  /* Chaque processus execute une boucle infinie */

  Initialisation();
  do
    :: ((token == true) && empty(canal_rec) && (requesting == true)) ->
       /* on oblige le detenteur du jeton a consulter les messages recus */
       in_SC:
       /* acces a la ressource critique (actions sur SharedVar), 
                  puis sortie de SC */
       printf("Proc%d in critical section !\n", id);
       SharedVar++;
       assert(SharedVar == 1); /* vérification EXO 2 ! */

       out_SC : 
       SharedVar--;
       printf("Proc%d leaved critical section !\n", id);
       Release_CS();
       
    :: canal_rec?typ_mes(val) ->
       /* traitement du message recu */ 
       if
	 :: (typ_mes == req) ->
	    Receive_Request_CS();
	 :: (typ_mes == tk) ->
	    Receive_Token();
	 :: else ->
	    printf("ERROR: Unknown message type !\n");
       fi;
    :: (requesting == false) -> /* demander la SC */
       ask_SC:
       Request_CS ();
  od;
}

/* Cree un ensemble de N processus */ 
init {
   byte proc; 
   atomic {
      proc=0;
      do
         :: proc <N ->
            run node(proc, 0); 
            proc++
         :: proc == N -> break 
      od
   }
}


ltl vivacite{always(dem_SC implies eventually en_SC)}
