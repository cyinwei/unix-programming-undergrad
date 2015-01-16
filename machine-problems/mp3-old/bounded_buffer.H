#ifndef _bounded_buffer_H
#define _bounded_buffer_H

#include "semaphore.H"
#include <pthread.h>
#include <queue>

class Bounded_buffer {
private:
	int total_size;
	Semaphore mutex;
	Semaphore full_count;
	Semaphore empty_count;
	std::queue<int> pipe;

public:
	Bounded_buffer(int _size);

	void produce(int item);
	int consume();
	int size() { return pipe.size(); }
};

#endif