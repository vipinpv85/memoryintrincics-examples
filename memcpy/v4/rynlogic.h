#ifndef __RUNLOGIC_H_
#define __RUNLOGIC_H_

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

#include <immintrin.h>
#include <x86intrin.h>
#include <pthread.h>

#define NUMTHREADS 8
#define NUMMEMGB 16
#define NUMVER 8
#define DISPLAYSTATS 1
#define TOTALSIZE (1UL << 34)

/*
 *
 * structure: myThreadData_t
 *
 * Elements:
 *      src             : hold src address
 *      dst             : hold dst address
 *      len             : total length
 *      threadIndex     : internal thread index
 *      tickCount       : to hold the ticks for various version runs
 *
 */
typedef struct myThreadData_s {
        long long unsigned tickCount;
        const char *src;
        const char *dst;
        size_t len;
        uint8_t threadIndex;
} myThreadData_t;

#endif /* __RUNLOGIC_H_ */
