#include <time.h>
#include <string.h>

#include "FastCpuCopy.h"

using namespace std;

int main(void)
{
	int numCPU = std::thread::hardware_concurrency();
        char *src = NULL;
        char *dst = NULL;

	std::cout << "--- FAST CPU MEMCPY & MEMCMP ---" << "\n";

	std::cout << " available CPU: " << numCPU << "\n";
	if (numCPU > 8) {
		MultiMemcpy test;

		src = new char[1UL << 34];
		if (src) {
			dst = new char[1UL << 34];
			if (dst) {
				std::cout << " Alloc memory via new for src &  dst " << std::endl;
			} else {
				std::cout << " Fail to alloc memory for dst " << dst << std::endl;
				delete[] src;
			}
		} else
			return 1;

		assert(src != NULL);
		assert(dst != NULL);

#if 1
 		void *ptr = memcpy((void *)dst, (void *)src, (1UL << 34));
 		assert(ptr == dst); /* to valdiate if copy has been completed */
		assert(memcmp(dst, src, (1UL << 34)) == 0);
#else
		test.Copy(src, dst, (1UL << 34));
#endif


		delete[] src;
		delete[] dst;
	}

	std::cout << "--------------------------------" << "\n";
	return 0;
}
