#ifndef __RUNLOGIC_H_
#define __RUNLOGIC_H_

#include <stdio.h>
#include <assert.h>

#include <immintrin.h>
#include <x86intrin.h>

#include <pthread.h>
#include <numa.h>
#include <assert.h>

#include <sys/mman.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

#include "vectorclass.h"

#define NUMTHREADS 8
#define NUMMEMGB 16
#define NUMVER 8
#define DISPLAYSTATS 1

/*
 *
 * structure: myThreadData_t
 *
 * Elements:
 *	src		: hold src address
 *	dst		: hold dst address
 *	len     	: total length
 *	threadIndex	: internal thread index
 *	tickCount	: to hold the ticks for various version runs
 *
 */
typedef struct myThreadData_s {
	long long unsigned tickCount[NUMVER];
	const char *src;
	const char *dst;
	size_t len;
	uint8_t threadIndex;
} myThreadData_t;

#ifdef __cplusplus
extern "C" {
#endif
void do_CopyCompare(const void *src, const void *dst, size_t len, uint8_t is_mmap);
#ifdef __cplusplus
}
#endif

#endif /* __RUNLOGIC_H_ */

