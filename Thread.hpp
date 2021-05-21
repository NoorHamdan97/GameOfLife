#ifndef __THREAD_H
#define __THREAD_H

#include "Headers.hpp"

class Thread
{
public:
	Thread(uint thread_id) : 
	m_thread_id(thread_id), m_thread(0)
	{
		// Only places thread_id 
	} 
	virtual ~Thread() {} // Does nothing 

	/** Returns true if the thread was successfully started, false if there was an error starting the thread */
	// Creates the internal thread via pthread_create 
	bool start()
	{
		int a = pthread_create(&m_thread, NULL, entry_func, this);
		if (a == 0) {
			return true;
		}
		return false;
	}

	/** Will not return until the internal thread has exited. */
	void join()
	{
		pthread_join(m_thread, NULL);
	}

	/** Returns the thread_id **/
	uint thread_id()
	{
		return m_thread_id;
	}
protected:
	// Implement this method in your subclass with the code you want your thread to run. 
	virtual void thread_workload() = 0;
	uint m_thread_id; // A number from 0 -> Number of threads initialized, providing a simple numbering for you to use

private:
	static void * entry_func(void * thread) { 
	((Thread *)thread)->thread_workload(); 
	return NULL; 
	}
	pthread_t m_thread;
};

#endif
