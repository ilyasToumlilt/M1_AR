# TP04 - Algorithme Phase
#
# @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
# @copyright (c) 2015, toumlilt
#
# @version 1.0
# @package toumlilt/M1/AR

CC=mpicc
BIN=bin
INC=include
LIB=lib
OBJ=obj
SRC=src


all: directories min_phase


directories: ${OBJ} ${BIN} ${LIB}

${OBJ}:
	mkdir ${OBJ}
${BIN}:
	mkdir ${BIN}
${LIB}:
	mkdir ${LIB}


# regles generales :
$(OBJ)/%.o: $(SRC)/%.c
	$(CC) -c -o $@ $< -I$(INC)

$(BIN)/% : $(OBJ)/%.o
	$(CC) -o $@ $<
#fin regles generales

#exo 1 :
min_phase: $(BIN)/min_phase
	mpirun -np 7 $(BIN)/min_phase

clean:
	rm -f ${OBJ}/* ${BIN}/* ${LIB}/*

cleanall:
	rm -rf ${OBJ} ${BIN} ${LIB}
	rm -f ${INC}/*~ ${SRC}/*~ *~

# auto-generated makefile by toumlilt-makegen