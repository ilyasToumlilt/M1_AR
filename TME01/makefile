# TP01 - Initiation à MPI avec OpenMPI
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


all: directories hello_world


directories: ${OBJ} ${BIN} ${LIB}

${OBJ}:
	mkdir ${OBJ}
${BIN}:
	mkdir ${BIN}
${LIB}:
	mkdir ${LIB}


# regles generales :
$(OBJ)/%.o: $(SRC)/%.c
	mpicc -c -o $@ $< -I$(INC)

$(BIN)/% : $(OBJ)/%.o
	mpicc -o $@ $<
#fin regles generales

#exo 2 :
hello_world: $(BIN)/hello_world
runexo2: hello_world
	mpirun -np 5 $(BIN)/hello_world

# exo 3.2 :
exo3_fournis: $(BIN)/exo3_fournis

# exo 3.3 :
exo3_edited: $(BIN)/exo3_edited

# exo 4.1 :
hello_neighbor: $(BIN)/hello_neighbor

# exo 4.2 :
hello_neighbor_2: $(BIN)/hello_neighbor_2

# exo 4.3 :
hello_neighbor_3: $(BIN)/hello_neighbor_3


clean:
	rm -f ${OBJ}/* ${BIN}/* ${LIB}/*

cleanall:
	rm -rf ${OBJ} ${BIN} ${LIB}
	rm -f ${INC}/*~ ${SRC}/*~ *~

