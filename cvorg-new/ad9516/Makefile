INCDIR := ../include

CFLAGS=	-Wall -g -I$(INCDIR)

LIBAD9516 = libad9516.a

all: $(LIBAD9516)

ad9516.o: 
	$(CC) $(CFLAGS) -c -I. ad9516.c -o $@

libad9516.a: ad9516.o
	$(AR) rv $(LIBAD9516) $^

clean:
	$(RM) *.a *.o $(BAKS)
