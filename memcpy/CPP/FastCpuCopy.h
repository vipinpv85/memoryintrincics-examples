#include <unistd.h>

#include <cstddef>
#include <thread>
#include <assert.h>
#include <iostream>
#include <ostream>
#include <string>


#define MULTIMEMCPY_THREADS 8

class MultiMemcpy: std::thread
{
public:
	MultiMemcpy();
	~MultiMemcpy();

	void Copy(void * dst, void * src, size_t size);

	struct Worker
	{
		unsigned int   id;
		bool           state;

		void  *dst;
		void  *src;
		size_t  size;
	};


private:

	struct Worker m_workers[MULTIMEMCPY_THREADS];
};

