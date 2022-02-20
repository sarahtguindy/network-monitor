CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILE1=interfaceMonitor.cpp
FILE2=networkMonitor.cpp

networkMonitor: $(FILE2)
	$(CC) $(CFLAGS) $(FILE2) -o $@  

interfaceMonitor: $(FILE1)
	$(CC) $(CFLAGS) $(FILE1) -o $@

all: networkMonitor interfaceMonitor

clean: 
	rm -f *.o networkMonitor interfaceMonitor