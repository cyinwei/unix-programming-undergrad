# makefile

all: dataserver simpleclient

reqchannel.o: reqchannel.H reqchannel.C
	g++ -c -g reqchannel.C

dataserver: dataserver.C reqchannel.o 
	g++ -g -o dataserver dataserver.C reqchannel.o -lpthread

semaphore.o: semaphore.H semaphore.C
	g++ -c -g semaphore.C

bounded_buffer.o: bounded_buffer.H bounded_buffer.C semaphore.o
	g++ -c -g bounded_buffer.C semaphore.o

simpleclient: simpleclient.C reqchannel.o semaphore.o bounded_buffer.o
	g++ -g -o simpleclient simpleclient.C reqchannel.o semaphore.o bounded_buffer.o -lpthread

