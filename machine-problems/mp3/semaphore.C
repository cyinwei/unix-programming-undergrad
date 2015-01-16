#include "semaphore.H"
#include <iostream>

using namespace std;

Semaphore::Semaphore() {
	value = 1;
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&c, NULL);
}

Semaphore::Semaphore(int _val) {
	value = _val;
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&c, NULL);
}

Semaphore::~Semaphore() {
	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&c);
}

/* The wait function */
int Semaphore::P() {
	/* accessing & altering shared resource value, need a lock */
	pthread_mutex_lock(&m);
	while(value <= 0) {
	/* use a while loop, avoids errors from using an if statement,
	   including that several threads can acquire the mutex with turns, and any one can modify value, 
	   which would be bad. */	
		pthread_cond_wait(&c, &m);
		/* no free resources, wait for the other thread to signal back */
	}
	value--;
	pthread_mutex_unlock(&m);
}

/* The signal function */
int Semaphore::V() {
	int previous_value;
	pthread_mutex_lock(&m);
	previous_value = value;
	value++;
	pthread_mutex_unlock(&m);

	/* we want to signal outside the mutex to 
	   increase concurrency, since mutex'd code is not
	   parallelizable */
	if (previous_value == 0) { 
		pthread_cond_signal(&c);
	}
}