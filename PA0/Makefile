CC = g++
CFLAGS= -g -Wall

all: myecho mycat

myecho: myecho.o
	$(CC) $(CFLAGS) myecho.o -o myecho 

myecho.o: myecho.cpp
	$(CC) $(CFLAGS) -c myecho.cpp

mycat: mycat.o
	$(CC) $(CFLAGS) mycat.o -o mycat

mycat.o: mycat.cpp
	$(CC) $(CFLAGS) -c mycat.cpp

clean:
	rm -rf *.o
	rm -rf mycat
	rm -rf myecho