CPU=L865

all: libipoctal

CFLAGS=-g -Wall

HEADERS := ./ 

libipoctal: 
	$(CC) $(CFLAGS) -c -I$(HEADERS) libipoctal.c -o libipoctal.o
	ar rcs libipoctal.a libipoctal.o

clean:
	rm -rf *.o libipoctal.a 
