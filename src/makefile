################################################################################
#	File: 	makefile
#	Author:	Kelley Neubauer
#	Date:	10/23/2020
#
#	Description:
#	This is a makefile for CS344 Assignment 3
#
################################################################################

all : smallsh

#
# Compiler
#
CXX = gcc

#
# Compiler Flags
#
# CXXFLAGS = -std=c99
CXXFLAGS = -std=gnu99
CXXFLAGS += -Wall 
CXXFLAGS += -pedantic-errors
CXXFLAGS += -g

#
# Project Name
#
PROJ = smallsh

#
# Source Files
#
SRC += main.c

#
# Header Files
#
# HEADER += 

#
# Object Files
#
OBJ += main.o

#
# Create Executable File
#
${PROJ}: ${OBJ} ${HEADER}
	${CXX} ${LDFLAGS} ${OBJ} -o ${PROJ}

#
# Create Object Files
#
${OBJ}: ${SRC}
	${CXX} ${CXXFLAGS} -c $(@:.o=.c)

#
# Clean Up
#
clean:
	rm *.o ${PROJ}