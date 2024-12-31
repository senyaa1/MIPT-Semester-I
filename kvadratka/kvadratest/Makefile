CC=gcc
CFLAGS=
NAME=kvadratest

SRCDIR=src

main: ${NAME}

prepare:
	mkdir -p ${OUTDIR} 
 
${NAME}: ${SRCDIR}/*.c
	${CC} -o $@ $^ ${CFLAGS}

clean:
	rm -rf ${NAME}

all: prepare ${NAME} 


.PHONY: all
