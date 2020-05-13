PROGS=httpserver clean
CFLAGS=-pthread -std=c++11

all:${PROGS}

httpserver:Server_main.o server.o unified.o thread_pool.o
	g++ -g -o httpserver Server_main.o server.o unified.o thread_pool.o ${CFLAGS}
Server_main.o:Server_main.cc
	g++ -g -c Server_main.cc ${CFLAGS}
server.o:server.cc
	g++ -g -c server.cc	 ${CFLAGS}
thread_pool.o:thread_pool.cc thread_pool.h
	g++ -g -c thread_pool.cc	 ${CFLAGS}
unified.o:unified.cc unified.h
	g++ -g -c unified.cc 

clean:
	rm -f *.o