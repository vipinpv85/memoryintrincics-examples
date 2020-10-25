#include <stdio.h>
#include <assert.h>
#include <signal.h>

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

static uint32_t numThreads = 8;
static uint32_t numMemGb = 16; /* in GB*/
static int quit = 0;
static pthread_t thread[1024];
static long long unsigned tickCount[64][8] __attribute__ ((aligned (16)));

typedef struct myThreadData_s {
	const char *src;
	const char *dst;
	size_t len;
	uint8_t threadIndex;
} myThreadData_t;

static void __attribute__ ((noinline)) v1_CopyCompare(const char * restrict src, const char * restrict dst, const size_t len)
{
	void *ptr = memcpy((void *)dst, (void *)src, len);

	//assert(ptr == (dst + len));
	printf(" ptr %p dst_len %p\n", ptr, (dst + len));
	assert(memcmp(dst, src, len) == 0);
}

static void __attribute__ ((noinline)) v2_CopyCompare(const char * restrict src, const char * restrict dst, const size_t len)
{
        unsigned long long int i = 0, j = 0;
        __m256i a01, a02, a03, a04, b01, b02, b03, b04;
        __m256i a11, a12, a13, a14, b11, b12, b13, b14;
        __m256i res01, res02, res03, res04, res11, res12, res13,  res14;

        const uint64_t vsrc[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)src +   0),
                (const uint64_t)((const char *)src +  32),
                (const uint64_t)((const char *)src +  64),
                (const uint64_t)((const char *)src +  96) };
        const uint64_t vsrc1[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)src + 128),
                (const uint64_t)((const char *)src + 160),
                (const uint64_t)((const char *)src + 192),
                (const uint64_t)((const char *)src + 224) };

        const uint64_t vdst[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)dst +  0),
                (const uint64_t)((const char *)dst + 32),
                (const uint64_t)((const char *)dst + 64),
                (const uint64_t)((const char *)dst + 96) };
        const uint64_t vdst1[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)dst + 128),
                (const uint64_t)((const char *)dst + 160),
                (const uint64_t)((const char *)dst + 192),
                (const uint64_t)((const char *)dst + 224) };

        __m256i vector_vsrc = _mm256_lddqu_si256((__m256i const *)vsrc);
        __m256i vector_vsrc1 = _mm256_lddqu_si256((__m256i const *)vsrc1);
        __m256i vector_vdst = _mm256_lddqu_si256((__m256i const *)vdst);
        __m256i vector_vdst1 = _mm256_lddqu_si256((__m256i const *)vdst1);
        __m256i vector_offset = _mm256_set_epi64x(128, 128, 128, 128);

        const uint64_t *sptr, *sptr1, *dptr, *dptr1;
        sptr = (const uint64_t *)&vector_vsrc;
        sptr1 = (const uint64_t *)&vector_vsrc1;
        dptr = (const uint64_t *)&vector_vdst;
        dptr1 = (const uint64_t *)&vector_vdst1;
        for (i = 0; i < (1UL << 31);  i += 256) {
                a01 = _mm256_lddqu_si256 ((__m256i const * )sptr[0]);
                a02 = _mm256_lddqu_si256 ((__m256i const * )sptr[1]);
                a03 = _mm256_lddqu_si256 ((__m256i const * )sptr[2]);
                a04 = _mm256_lddqu_si256 ((__m256i const * )sptr[3]);
                a11 = _mm256_lddqu_si256 ((__m256i const * )sptr1[0]);
                a12 = _mm256_lddqu_si256 ((__m256i const * )sptr1[1]);
                a13 = _mm256_lddqu_si256 ((__m256i const * )sptr1[2]);
                a14 = _mm256_lddqu_si256 ((__m256i const * )sptr1[3]);
                _mm256_storeu_si256((__m256i *)dptr[0], a01);
                _mm256_storeu_si256((__m256i *)dptr[1], a02);
                _mm256_storeu_si256((__m256i *)dptr[2], a03);
                _mm256_storeu_si256((__m256i *)dptr[3], a04);
                _mm256_storeu_si256((__m256i *)dptr1[0], a11);
                _mm256_storeu_si256((__m256i *)dptr1[1], a12);
                _mm256_storeu_si256((__m256i *)dptr1[2], a13);
                _mm256_storeu_si256((__m256i *)dptr1[3], a14);

                vector_vsrc = _mm256_add_epi64 (vector_vsrc, vector_offset);
                vector_vsrc1 = _mm256_add_epi64 (vector_vsrc, vector_offset);
                vector_vdst = _mm256_add_epi64 (vector_vdst, vector_offset);
                vector_vdst1 = _mm256_add_epi64 (vector_vdst, vector_offset);
                sptr = (const uint64_t *)&vector_vsrc;
                sptr1 = (const uint64_t *)&vector_vsrc1;
                dptr = (const uint64_t *)&vector_vdst;
                dptr1 = (const uint64_t *)&vector_vdst1;
        }
        for (i = 0; i < (1UL << 31);  i += 256) {
                a01 = _mm256_lddqu_si256 ((__m256i const * )sptr[0]);
                a02 = _mm256_lddqu_si256 ((__m256i const * )sptr[1]);
                a03 = _mm256_lddqu_si256 ((__m256i const * )sptr[2]);
                a04 = _mm256_lddqu_si256 ((__m256i const * )sptr[3]);
                a11 = _mm256_lddqu_si256 ((__m256i const * )sptr1[0]);
                a12 = _mm256_lddqu_si256 ((__m256i const * )sptr1[1]);
                a13 = _mm256_lddqu_si256 ((__m256i const * )sptr1[2]);
                a14 = _mm256_lddqu_si256 ((__m256i const * )sptr1[3]);
                b01 = _mm256_lddqu_si256 ((__m256i const * )dptr[0]);
                b02 = _mm256_lddqu_si256 ((__m256i const * )dptr[1]);
                b03 = _mm256_lddqu_si256 ((__m256i const * )dptr[2]);
                b04 = _mm256_lddqu_si256 ((__m256i const * )dptr[3]);
                b11 = _mm256_lddqu_si256 ((__m256i const * )dptr1[0]);
                b12 = _mm256_lddqu_si256 ((__m256i const * )dptr1[1]);
                b13 = _mm256_lddqu_si256 ((__m256i const * )dptr1[2]);
                b14 = _mm256_lddqu_si256 ((__m256i const * )dptr1[3]);

                vector_vsrc = _mm256_add_epi64 (vector_vsrc, vector_offset);
                vector_vsrc1 = _mm256_add_epi64 (vector_vsrc, vector_offset);
                vector_vdst = _mm256_add_epi64 (vector_vdst, vector_offset);
                vector_vdst1 = _mm256_add_epi64 (vector_vdst, vector_offset);
                sptr = (const uint64_t *)&vector_vsrc;
                sptr1 = (const uint64_t *)&vector_vsrc1;
                dptr = (const uint64_t *)&vector_vdst;
                dptr1 = (const uint64_t *)&vector_vdst1;

                const __m256i res01 = _mm256_xor_si256(a01,b01);
                const __m256i res02 = _mm256_xor_si256(a02,b02);
                const __m256i res03 = _mm256_xor_si256(a03,b03);
                const __m256i res04 = _mm256_xor_si256(a04,b04);
                const __m256i res11 = _mm256_xor_si256(a11,b11);
                const __m256i res12 = _mm256_xor_si256(a12,b12);
                const __m256i res13 = _mm256_xor_si256(a13,b13);
                const __m256i res14 = _mm256_xor_si256(a14,b14);

                assert((_mm256_testz_si256(res01, res02) & _mm256_testz_si256(res03, res04)) == 1);
                assert((_mm256_testz_si256(res11, res12) & _mm256_testz_si256(res13, res14)) == 1);
        }
}


static void __attribute__ ((noinline)) v8_CopyCompare(const char * restrict src, const char * restrict dst, const size_t len)
{
	unsigned long long int i = 0, j = 0;
	__m256i a01, a02, a03, a04, b01, b02, b03, b04;
	__m256i a11, a12, a13, a14, b11, b12, b13, b14;
	__m256i res01, res02, res03, res04, res11, res12, res13,  res14;

	const uint64_t vsrc[4]  __attribute__ ((aligned (16)))= {
		(const uint64_t)((const char *)src +   0),
		(const uint64_t)((const char *)src +  32),
		(const uint64_t)((const char *)src +  64),
		(const uint64_t)((const char *)src +  96) }; 
	const uint64_t vsrc1[4]  __attribute__ ((aligned (16)))= {
		(const uint64_t)((const char *)src + 128),
		(const uint64_t)((const char *)src + 160),
		(const uint64_t)((const char *)src + 192),
		(const uint64_t)((const char *)src + 224) }; 

	const uint64_t vdst[4]  __attribute__ ((aligned (16)))= {
		(const uint64_t)((const char *)dst +  0),
		(const uint64_t)((const char *)dst + 32),
		(const uint64_t)((const char *)dst + 64),
		(const uint64_t)((const char *)dst + 96) }; 
	const uint64_t vdst1[4]  __attribute__ ((aligned (16)))= {
		(const uint64_t)((const char *)dst + 128),
		(const uint64_t)((const char *)dst + 160),
		(const uint64_t)((const char *)dst + 192),
		(const uint64_t)((const char *)dst + 224) }; 

	__m256i vector_vsrc = _mm256_lddqu_si256((__m256i const *)vsrc);
	__m256i vector_vsrc1 = _mm256_lddqu_si256((__m256i const *)vsrc1);
	__m256i vector_vdst = _mm256_lddqu_si256((__m256i const *)vdst);
	__m256i vector_vdst1 = _mm256_lddqu_si256((__m256i const *)vdst1);
	__m256i vector_offset = _mm256_set_epi64x(128, 128, 128, 128);

	const uint64_t *sptr, *sptr1, *dptr, *dptr1;
	sptr = (const uint64_t *)&vector_vsrc;
	sptr1 = (const uint64_t *)&vector_vsrc1;
	dptr = (const uint64_t *)&vector_vdst;
	dptr1 = (const uint64_t *)&vector_vdst1;

	for (i = 0; i < (1UL << 31);  i += 256) {
		a01 = _mm256_lddqu_si256 ((__m256i const * )sptr[0]);
		a02 = _mm256_lddqu_si256 ((__m256i const * )sptr[1]);
		a03 = _mm256_lddqu_si256 ((__m256i const * )sptr[2]);
		a04 = _mm256_lddqu_si256 ((__m256i const * )sptr[3]);
		a11 = _mm256_lddqu_si256 ((__m256i const * )sptr1[0]);
		a12 = _mm256_lddqu_si256 ((__m256i const * )sptr1[1]);
		a13 = _mm256_lddqu_si256 ((__m256i const * )sptr1[2]);
		a14 = _mm256_lddqu_si256 ((__m256i const * )sptr1[3]);
		_mm256_storeu_si256((__m256i *)dptr[0], a01);
		_mm256_storeu_si256((__m256i *)dptr[1], a02);
		_mm256_storeu_si256((__m256i *)dptr[2], a03);
		_mm256_storeu_si256((__m256i *)dptr[3], a04);
		_mm256_storeu_si256((__m256i *)dptr1[0], a11);
		_mm256_storeu_si256((__m256i *)dptr1[1], a12);
		_mm256_storeu_si256((__m256i *)dptr1[2], a13);
		_mm256_storeu_si256((__m256i *)dptr1[3], a14);
		b01 = _mm256_lddqu_si256 ((__m256i const * )dptr[0]);
		b02 = _mm256_lddqu_si256 ((__m256i const * )dptr[1]);
		b03 = _mm256_lddqu_si256 ((__m256i const * )dptr[2]);
		b04 = _mm256_lddqu_si256 ((__m256i const * )dptr[3]);
		b11 = _mm256_lddqu_si256 ((__m256i const * )dptr1[0]);
		b12 = _mm256_lddqu_si256 ((__m256i const * )dptr1[1]);
		b13 = _mm256_lddqu_si256 ((__m256i const * )dptr1[2]);
		b14 = _mm256_lddqu_si256 ((__m256i const * )dptr1[3]);

		vector_vsrc = _mm256_add_epi64 (vector_vsrc, vector_offset);
		vector_vsrc1 = _mm256_add_epi64 (vector_vsrc, vector_offset);
		vector_vdst = _mm256_add_epi64 (vector_vdst, vector_offset);
		vector_vdst1 = _mm256_add_epi64 (vector_vdst, vector_offset);
		sptr = (const uint64_t *)&vector_vsrc;
		sptr1 = (const uint64_t *)&vector_vsrc1;
		dptr = (const uint64_t *)&vector_vdst;
		dptr1 = (const uint64_t *)&vector_vdst1;

		const __m256i res01 = _mm256_xor_si256(a01,b01);
		const __m256i res02 = _mm256_xor_si256(a02,b02);
		const __m256i res03 = _mm256_xor_si256(a03,b03);
		const __m256i res04 = _mm256_xor_si256(a04,b04);
		const __m256i res11 = _mm256_xor_si256(a11,b11);
		const __m256i res12 = _mm256_xor_si256(a12,b12);
		const __m256i res13 = _mm256_xor_si256(a13,b13);
		const __m256i res14 = _mm256_xor_si256(a14,b14);

		assert((_mm256_testz_si256(res01, res02) & _mm256_testz_si256(res03, res04)) == 1);
		assert((_mm256_testz_si256(res11, res12) & _mm256_testz_si256(res13, res14)) == 1);
	}
}



static void *do_CopyCompare(void *userData)
{
	myThreadData_t *threadData = (myThreadData_t *)userData;
	uint64_t start, stop;
	unsigned int ignore;

	int threadIndex = ((myThreadData_t *)userData)->threadIndex;

	//while((__sync_fetch_and_and(&quit, 0) == 0));
	
	fprintf(stdout," Enter do_CopyCompare for %d!\n", threadIndex);
	do {
		/* v1 */
	        start = __rdtscp(&ignore);
		v1_CopyCompare(threadData->src, threadData->dst, threadData->len);
	        stop = __rdtscp(&ignore);
		tickCount[threadIndex][0] = stop - start;

		/* v8 */
	        start = __rdtscp(&ignore);
		v8_CopyCompare(threadData->src, threadData->dst, threadData->len);
	        stop = __rdtscp(&ignore);
		tickCount[threadIndex][7] = stop - start;

	} while(0);
	fprintf(stdout," Exit do_CopyCompare %d!\n", threadIndex);
}

static void signal_handler(int signum)
{
	fprintf(stdout, " threads signalled to quit!!!\n");
	__sync_fetch_and_or(&quit, 2);
}

int main(int argc, char **argv)
{
	uint8_t is_mmap = 0;
	char *src;
	char *dst;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	assert(numa_available() == 0);
	assert(numa_num_configured_cpus() >= numThreads);

	printf("\033[2J");
	printf(" we have %u CPU, run test!", numa_num_configured_cpus());
	printf(" RUN: CPU requested (%u)\n", numThreads);
	printf(" RUN: MEMORY requested (%u GB)\n", numMemGb * 2);

	/* check if we can equally divide this among availble cores */
	if (numMemGb % numThreads) {
		fprintf(stderr, "ERR: suggest memory in multiple of threads!\n");
		exit(1);
	}

	for (int i = 0; i < argc; i++)
		fprintf(stdout, " index-%d: (%s)\n", i, *argv);

	/* allocate src with 16GB */
	/* allocate dst with 16GB */
	src = malloc(sizeof(char) * 1024 * 1024 * 1024 * numMemGb);
	if (src != NULL) {
		dst = malloc(sizeof(char) * 1024 * 1024 * 1024 * numMemGb);
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

		src = mmap(NULL, (1UL << 34), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
		dst = mmap(NULL, (1UL << 34), PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_NORESERVE, -1, 0);

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

	fprintf(stdout, "src %p dst %p \n", src, dst);
	myThreadData_t userData[8] __attribute__ ((aligned (16)));

	for (uint8_t i = 0; i < 8; i++) {
		userData[i].src = src + (1UL << 31) * i;
		userData[i].dst = dst + (1UL << 31) * i;
		userData[i].len = (1UL << 31);
		userData[i].threadIndex = i;
	}

	/* allocate copy & compare */
	for (int k = 0; k < numThreads; k++) {
		assert(pthread_create(&thread[k], NULL, do_CopyCompare, (void*)&userData[k]) == 0);
	}

	for (int k = 0; k < numThreads; k++)
		pthread_join(thread[k], NULL);

	/* display stats */
	printf("\033[2J");
        printf("\033[1;1H ---- Summary in ticks -----");
	int i = 0;
	long long unsigned total[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        for (; i < numThreads; i++) {
                printf("\033[%d;1H Thread-%03d: v1 (%10llu), v8(%10llu)",
                        i + 2, i, tickCount[i][0], tickCount[i][7]);
		total[0] += tickCount[i][0];
		total[7] += tickCount[i][7];
	}
        printf("\033[%d;1H --- with Pstate ticks can be unreliable ---", i + 2);
	printf("\033[%d;1H Total: %llu, %llu with (%s)",
		i + 3, total[0], total[7], (is_mmap)?"mmap":"malloc");
        printf("\033[%d;1H speedup %%: %f, %f",
		i + 4,
		((float)(total[0] - 0) * 100)/ (float)total[0],
		((float)(total[0] - total[7]) * 100)/ (float)total[7]
		);
        printf("\033[%d;1H ---------------------------------", i + 5);
	
        printf("\033[%d;1H", i + 5);
	printf("\n");

	/* free src with 16GB */
	/* free dst with 16GB */
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
