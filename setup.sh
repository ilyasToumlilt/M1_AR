#!/bin/bash

# la construction de l'executable necessite une edition dynamique de liens
# avec les bibliotheques de openmpi :
LD_LIBRARY_PATH=/usr/local/lib
export LD_LIBRARY_PATH

# openmpi utilise ssh pour que les différentes machines de l'application
# puissent converser. Pour éviter de taper votre mot de passe à chaque
# fois que vous lancez un programme MPI, créez, si ce n'est déjà fait,
# un couple de clés ssh grâce à la commande ssh-keygen et ajoutez-la
# aux clés autorisées.
# ssh-keygen -q -N '' -f ~/.ssh/id_rsa
# cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
# vérifiez en tapant la commande :
# ssh localhost
