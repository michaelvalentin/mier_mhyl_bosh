all: bosh

OBJS = parser.o print.o
LIBS= -lreadline -ltermcap
CC = gcc

bosh: bosh.o ${OBJS}
	${CC} -o $@  ${LIBS} bosh.o ${OBJS}

clean:
	rm -rf *o bosh
