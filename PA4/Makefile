CC = gcc
CCOPTS = -c -g -Wall
LINKOPTS = -g -lpthread -lrt -Wall

EXEC=scheduler
OBJECTS=scheduler.o worker.o list.o smp5_tests.o testrunner.o

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) -o $@ $^ $(LINKOPTS)

%.o:%.c
	$(CC) $(CCOPTS) -o $@ $^

clean:
	- $(RM) $(EXEC)
	- $(RM) $(OBJECTS)
	- $(RM) *~
	- $(RM) core.*

test: scheduler
	- ./scheduler -test -f0 rr
	- killall -q -KILL scheduler; true

pretty: 
	indent *.c *.h -kr
