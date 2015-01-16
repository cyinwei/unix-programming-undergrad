#include "semaphore.H"
#include <vector>

class BoundedBuffer {
private:
	Semaphore mutex;
	Semaphore hasItems;
	Semaphore hasSpace;
	std::vector<int> values;
	std::vector<int> people;

public:
	BoundedBuffer(int size);
	/* implicit destructor is enough the cleanly exit */

	void produce(int person, int value);
	std::vector<int> consume();
	//bool empty();
};