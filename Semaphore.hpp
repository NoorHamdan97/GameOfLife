#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H
#include <stdio.h>
#include "Headers.hpp"

class Semaphore {
private:
	pthread_mutexattr_t attr;
	pthread_mutex_t locker;
	pthread_cond_t cond_var;
	unsigned value;
public:
	Semaphore() :
			cond_var(
					PTHREAD_COND_INITIALIZER), value(0) {
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&locker, &attr);
	} // Constructs a new semaphore with a counter of 0
	Semaphore(unsigned val) : locker(PTHREAD_MUTEX_INITIALIZER), cond_var(
			PTHREAD_COND_INITIALIZER), value(val) {
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&locker, &attr);
	} // Constructs a new semaphore with a counter of val


	void down(){
	pthread_mutex_lock(&(this->locker));
	while (value==0){
		pthread_cond_wait(&(this->cond_var),&(this->locker));
	}
	this->value--;
	pthread_mutex_unlock(&(this->locker));
}

void up(){
	pthread_mutex_lock(&(this->locker));
	this->value++;
	pthread_cond_broadcast(&(this->cond_var));
	pthread_mutex_unlock(&(this->locker));
}


};

#endif
