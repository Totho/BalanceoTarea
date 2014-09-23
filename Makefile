CC=g++ -std=c++11
#CC=clang++ -std=c++11
ZMQ=/home/utp/cs/zmq
#ZMQ=/usr/local
ZMQ_LIBS=$(ZMQ)/lib
ZMQ_HDRS=$(ZMQ)/include


all: client server worker

client: client.o
	$(CC) -L$(ZMQ_LIBS) -o client client.o -lzmq -lczmq

client.o: client.cc 
	$(CC) -I$(ZMQ_HDRS) -c client.cc 

server: server.o
	$(CC) -L$(ZMQ_LIBS) -o server server.o -lzmq -lczmq

server.o: server.cc 
	$(CC) -I$(ZMQ_HDRS)  -c server.cc 

worker: worker.o
	$(CC) -L$(ZMQ_LIBS) -o worker worker.o -lzmq -lczmq

worker.o: worker.cc 
	$(CC) -I$(ZMQ_HDRS)  -c worker.cc 

clean:
	rm -rf client client.o server server.o worker worker.o *~
