#include "bounded_buffer.H"
#include <iostream>

/* Implementation of bounded_buffer.H
 */

Bounded_buffer::Bounded_buffer(int _size) {
	total_size = _size;
	full_count = Semaphore(0);
	empty_count = Semaphore(_size);
	mutex = Semaphore(1);
	std::cout << "we in bounded buffer boyz, "<< full_count.value << empty_count.value << "wat" << mutex.value << std::endl << std::flush;
}

/* Strategy:
 *   use 2 semaphores, one for produce function and one for the
 *   consume function.
 */
 
void Bounded_buffer::produce(int item) {
	empty_count.P();
	mutex.P();
	pipe.push(item);
	mutex.V();
	full_count.V();
	std::cout<<"in produce with item "<<item<<std::endl<<std::flush;
	return;
}

int Bounded_buffer::consume() {
	full_count.P();
	mutex.P();
	int product = pipe.front();
	pipe.pop();
	mutex.V();
	empty_count.V();
	std::cout<<"in consume with product."<<product<<std::endl<<std::flush;
	return product;
}