################################################################################
#	File: 	makefile
#	Author:	Kelley Neubauer
#	Date:	10/23/2020
#
#	Description: makefile for kell-shell
#
################################################################################

all : kell-shell

#
# Compiler
#
CC = gcc

#
# Compiler Flags
#
CFLAGS = -std=gnu99
CFLAGS += -Wall 
CFLAGS += -pedantic-errors
CFLAGS += -g

#
# Project Name
#
PROJ = kell-shell

#
# Source Files
#
SRC += main.c

#
# Object Files
#
OBJ += main.o

#
# Create Executable File
#
${PROJ}: ${OBJ} ${HEADER}
	${CC} ${LFLAGS} ${OBJ} -o ${PROJ}

#
# Create Object Files
#
${OBJ}: ${SRC}
	${CC} ${CFLAGS} -c $(@:.o=.c)

#
# Clean Up
#
clean:
	rm *.o ${PROJ}
