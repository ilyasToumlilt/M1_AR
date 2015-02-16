# TP03 - Minimum sur un arbre et echo avec MPI
#
# @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
# @copyright (c) 2015, toumlilt
#
# @version 1.2
# @package toumlilt/M1/AR

CC=gcc -Wall -ansi
BIN=bin
INC=include
LIB=lib
OBJ=obj
SRC=src


all: directories min_abr min_echo


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
min_abr: $(BIN)/min_abr
	mpirun -np 7 $(BIN)/min_abr

min_echo: $(BIN)/min_echo
	mpirun -np 7 $(BIN)/min_echo

clean:
	rm -f ${OBJ}/* ${BIN}/* ${LIB}/*

cleanall:
	rm -rf ${OBJ} ${BIN} ${LIB}
	rm -f ${INC}/*~ ${SRC}/*~ *~

