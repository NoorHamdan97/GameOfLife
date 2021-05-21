#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Semaphore.hpp"

// Single Producer - Multiple Consumer queue
template <typename T>class PCQueue
{

public:

	PCQueue() :
			push_allowed(
			PTHREAD_COND_INITIALIZER), pop_allowed(
			PTHREAD_COND_INITIALIZER), producer_inside(0), consumer_inside(0), producer_waiting(
					0), size(0) {
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&locker, &attr);
	}


	// Blocks while queue is empty. When queue holds items, allows for a single
	// thread to enter and remove an item from the front of the queue and return it.
	// Assumes multiple consumers.
	T pop(){
		pthread_mutex_lock(&(locker));
		while(producer_inside || consumer_inside || producer_waiting || !size){
			pthread_cond_wait(&pop_allowed,&locker);
		}
		consumer_inside = 1;
		T item = items.front();
		items.pop();
		size--;
		consumer_inside = 0;
		if(producer_waiting){
			pthread_cond_signal(&push_allowed);
		} else{
			pthread_cond_signal(&pop_allowed);
		}
		pthread_mutex_unlock(&(locker));
		return item;
	}

	// Allows for producer to enter with *minimal delay* and push items to back of the queue.
	// Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
	// Assumes single producer
	void push(const T& item){
		pthread_mutex_lock(&(locker));
		producer_waiting = 1;
		while(producer_inside || consumer_inside){
			pthread_cond_wait(&push_allowed,&locker);
		}
		producer_inside = 1;
		producer_waiting = 0;
		items.push(item);
		size++;
		producer_inside = 0;
		pthread_cond_signal(&pop_allowed);
		pthread_mutex_unlock(&(locker));
	}


private:
	std::queue<T,std::list<T>> items;
	pthread_mutexattr_t attr;
	pthread_mutex_t locker;
	pthread_cond_t push_allowed;
	pthread_cond_t pop_allowed;
	int producer_inside, consumer_inside, producer_waiting, size;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif
