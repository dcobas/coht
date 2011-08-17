YACCDIR = .
CFLAGS = -I. -I${YACCDIR}

all: conv.tab.c tg8pars

conv.tab.c: conv.y convP.c convP.h ${YACCDIR}/yacc.c ${YACCDIR}/yacc.h
	bison conv.y

tg8pars: conv.tab.c
	gcc conv.tab.c ${CFLAGS} -o tg8pars
