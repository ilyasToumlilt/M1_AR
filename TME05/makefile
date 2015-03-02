# TP05 - Implantation sous MPI de l'algo de Ricart & Agrawala
#
# @author Ilyas Toumlilt <toumlilt.ilyas@gmail.com>
#
# @version 1.0
# @package toumlilt/M1/AR

CC=mpicc
BIN=bin
INC=include
LIB=lib
OBJ=obj
SRC=src


all: directories ricart_agrawala


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
	$(CC) -o $@ $< -lpthread
#fin regles generales

ricart_agrawala: $(BIN)/ricart_agrawala

clean:
	rm -f ${OBJ}/* ${BIN}/* ${LIB}/*

cleanall:
	rm -rf ${OBJ} ${BIN} ${LIB}
	rm -f ${INC}/*~ ${SRC}/*~ *~

# auto-generated makefile by toumlilt-makegen
