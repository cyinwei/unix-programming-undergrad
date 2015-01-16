#include "bounded_buffer.H"
#include <vector>
#include <iostream>

using namespace std;

BoundedBuffer::BoundedBuffer(int size) {
	mutex = Semaphore(1);
	hasSpace = Semaphore(size);
	hasItems = Semaphore(0);
}

void BoundedBuffer::produce(int person, int value) {
	cout<<"\nIn BoundedBuffer::produce()\n"<<flush;
	hasSpace.P(); 
	mutex.P();
	people.push_back(person);
	values.push_back(value);
	mutex.V();
	hasItems.V();
}

std::vector<int> BoundedBuffer::consume() {
	cout<<"\nIn BoundedBuffer::consume()\n"<<flush;
	int return_person, return_value;
	vector<int> return_data (2);

	hasItems.P();
	cout<<"\nIn BoundedBuffer::consume(), past hasItems lock...\n"<<flush;
	mutex.P();
	return_person = people[people.size()-1];
	return_value = values[values.size()-1];
	people.pop_back();
	values.pop_back();
	mutex.V();
	hasSpace.V();

	return_data[0] = return_person;
	return_data[1] = return_value;
	return return_data;
}

// bool BoundedBuffer::empty() {
// 	bool empty;
// 	mutex.P();
// 	if ()
// }