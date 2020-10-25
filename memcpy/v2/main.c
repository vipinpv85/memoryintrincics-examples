#include "runlogic.h"

#include <numa.h>
#include <assert.h>

#include <sys/mman.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

int main(int argc, char **argv)
{
	uint8_t is_mmap = 0;
	char *src;
	char *dst;

	assert(numa_available() == 0);
	assert(numa_num_configured_cpus() >= NUMTHREADS);

	printf("\033[2J");
	fprintf(stdout, " we have %u CPU, run test!", numa_num_configured_cpus());
	fprintf(stdout, " RUN: CPU requested (%u)\n", NUMTHREADS);
	fprintf(stdout, " RUN: MEMORY requested (%u GB)\n", NUMMEMGB * 2);

	/* check if we can equally divide this among availble cores */
	if (NUMMEMGB % NUMTHREADS) {
		fprintf(stderr, "ERR: suggest memory in multiple of threads!\n");
		exit(1);
	}

	for (int i = 0; i < argc; i++)
		fprintf(stdout, " index-%d: (%s)\n", i, *argv);

	/* allocate src with 16GB */
	/* allocate dst with 16GB */
	src = (char *)malloc(sizeof(char) * 1024 * 1024 * 1024 * NUMMEMGB);
	if (src != NULL) {
		dst = (char *)malloc(sizeof(char) * 1024 * 1024 * 1024 * NUMMEMGB);
		if (dst != NULL)
			fprintf(stdout, " allocated 16GB src and 16GB dst with malloc!\n");
		else {
			fprintf(stderr, "FAIL to allocate dst from malloc using mmap\n");
			free(src);
			src = NULL;
			dst = NULL;
		}
	} else {
		fprintf(stderr, "FAIL to allocate src from malloc using mmap\n");
		src = NULL;
		dst = NULL;
	}

	if ((src == NULL) && (dst == NULL)) {
		is_mmap = 1;

		src = (char *)mmap(NULL, (1UL << 34), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
		dst = (char *)mmap(NULL, (1UL << 34), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_NORESERVE, -1, 0);

		if ((src == MAP_FAILED) || (dst == MAP_FAILED)) {
			fprintf(stderr, "ERR: src %p dst %p MAP_FAILED\n", src, dst);
			exit(2);
		}
	}

	assert(src);
	assert(dst);

#if FILLRNDM
	/* fill src with random value */
        int rnd=open("/dev/urandom", O_RDONLY);
        int64_t readSize = 0, temp = 0;
	do {
        	temp = read(rnd, src + readSize, (1UL << 34) - readSize);
		if (temp == -1) {
			fprintf(stderr, "ERR: temp %lu readSize %lu \n", temp, readSize);
		}
		readSize += temp;
		fprintf(stdout, " current read %lu temp %lu target %lu\n", readSize, temp, 1UL << 34);
	} while(readSize < (1UL << 34));
        close(rnd);
#endif

	fprintf(stdout, " %s src %p dst %p \n", (is_mmap)?"MMAP":"MALLOC",  src, dst);
	do_CopyCompare(src, dst, (1UL << 34), is_mmap);

	/* free src with 16GB */
	/* free dst with 16GB */
	fprintf(stdout, " freeing up %s src %p dst %p \n", (is_mmap)?"MMAP":"MALLOC", src, dst);
	if (is_mmap) {
		unlink("src");
		unlink("dst");
		munmap(src, (1UL << 34));
		munmap(dst, (1UL << 34));
	} else {
		free(src);
		free(dst);
	}

	return 0;
}
