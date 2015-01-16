/* Implementation of Semaphore.H
 * 
 */

#include "semaphore.H"
#include <stdio.h>

Semaphore::Semaphore() {
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&c, NULL);
	value = 1;
}

Semaphore::Semaphore(int _val) {
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&c, NULL);
	value = _val;
}

Semaphore::~Semaphore() {
	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&c);
}

/* The wait function.
 */
int Semaphore::P() {
	pthread_mutex_lock(&m);
	while (value > 0) {
	/* There are free resources available.
	 *
	 * Note the use of while() instead of if().
	 * This is to deal with multiple waiting threads, and
	 * spurious wakeups.
	 */	
		value--;
		pthread_mutex_unlock(&m);
		return 0;
	}
	

	/* No free resources.  Let the current thread wait for an
	 * available resource.
	 */
	pthread_cond_wait(&c, &m); 
	/* note pthread_cond_wait() *automatically* unlocks the mutex, 
	 * and halts the current thread, waiting for a signal() or broadcast() 
	 * [single vs. multiple waiting threads].
	 *
	 * After the thread continues, we **must manually** unlock the mutex.
	 */

	value --; //we're consuming the resource now that the thread is running again.
	pthread_mutex_unlock(&m);
	return 0;

	/* Shouldn't ever get here... */
	fprintf(stderr, "Error:: In semaphore.C, function P(), value out bounds.\n");
	return -1;
}


/* The signal function.
 */
int Semaphore::V() {
	pthread_mutex_lock(&m);
	value += 1;
	/* We're freeing a shared resource.  
	 */
	pthread_cond_broadcast(&c);
	/* Letting any waiting threads know.
	 */
	pthread_mutex_unlock(&m);
	return 0;
}

