# TP02 - Implantation sous MPI d'horloges de Lamport
#
# @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
# @copyright (c) 2015, toumlilt
#
# @version 1.0
# @package toumlilt/M1/AR

CC=gcc -Wall -ansi
BIN=bin
INC=include
LIB=lib
OBJ=obj
SRC=src
LPTHREAD = -lpthread

all: directories philosophers_diner


directories: ${OBJ} ${BIN} ${LIB}

${OBJ}:
	mkdir ${OBJ}
${BIN}:
	mkdir ${BIN}
${LIB}:
	mkdir ${LIB}


# regles generales :
$(OBJ)/%.o: $(SRC)/%.c
	mpicc -c -o $@ $< -I$(INC) -DVERBOSE

$(BIN)/% : $(OBJ)/%.o
	mpicc -o $@ $< ${LPTHREAD}
#fin regles generales

# without server
philosophers_diner: $(BIN)/philosophers_diner

# with server
# comming soon



clean:
	rm -f ${OBJ}/* ${BIN}/* ${LIB}/*

cleanall:
	rm -rf ${OBJ} ${BIN} ${LIB}
	rm -f ${INC}/*~ ${SRC}/*~ *~

