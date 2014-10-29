CC=gcc
DEPS = sp.h
#CFLAGS = -ansi -c -Wall -pedantic 
CFLAGS = -c  -g
SP_LIBRARY_DIR = /home/cs437/exercises/ex3
all: mcast

mcast: mcast.o $(DEPS) 
	    $(CC) -o mcast mcast.o $(SP_LIBRARY_DIR)/libspread-core.a -ldl -lm -lrt -lnsl $(SP_LIBRARY_DIR)/libspread-util.a 
clean:
	rm *.o
	rm mcast

%.o:    %.c
	$(CC) $(CFLAGS) $*.c

