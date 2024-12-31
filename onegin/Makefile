CC=gcc
CFLAGS=-D _DEBUG -Wall

NAME=onegin

SRCDIR=src
OUTDIR=bin

main: ${OUTDIR}/${NAME} 
 
${OUTDIR}/${NAME}: ${SRCDIR}/*.c
	${CC} -o $@ $^ ${CFLAGS}

prepare:
	mkdir -p ${OUTDIR} 

clean:
	rm -rf ${OUTDIR}/*

all: prepare main 

.PHONY: all
