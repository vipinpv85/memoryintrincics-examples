#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <immintrin.h>
#include <pthread.h>
#include <numa.h>

#include <sys/mman.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

static uint32_t numThreads = 8;
static uint32_t numMemGb = 16; /* in GB*/
static int quit = 0;
static pthread_t thread[8];
static long long unsigned tickCount[1024][7] __attribute__ ((aligned (16)));
static sigset_t signal_mask;

char *src;
char *dst;

static void __attribute__ ((noinline)) v1_CopyCompare(const int offsetIndex)
{
	void *ptr = memcpy(dst + offsetIndex * (1UL << 31),
		src + offsetIndex * (1UL << 31),
		(1UL << 31));

	assert(ptr == (dst + offsetIndex * (1UL << 31)));
	assert(memcmp(dst + offsetIndex * (1UL << 31), src + offsetIndex * (1UL << 31), (1UL << 31)) == 0);
}

static void __attribute__ ((noinline)) v2_CopyCompare(const int offsetIndex)
{
	const unsigned long long int pseudoIndex = offsetIndex * (1UL << 31);
	const char *vsrc1 = (const char *)src + pseudoIndex;
	const char *vsrc2 = (const char *)src + pseudoIndex + 32;
	const char *vdst1 = (const char *)dst + pseudoIndex;
	const char *vdst2 = (const char *)dst + pseudoIndex + 32;

	for (unsigned long long int i = 0; i < (1UL << 31); i += 64)
	{
		const __m256i a1 = _mm256_loadu_si256((__m256i const * )(vsrc1 + i));
		const __m256i a2 = _mm256_loadu_si256((__m256i const * )(vsrc2 + i));
		_mm256_storeu_si256((__m256i *)(vdst1 + i), a1);
		_mm256_storeu_si256((__m256i *)(vdst2 + i), a2);
	}

	for (unsigned long long int i = 0; i < (1UL << 31); i += 64)
	{
		const __m256i a1 = _mm256_loadu_si256((__m256i const * )(vsrc1 + i));
		const __m256i a2 = _mm256_loadu_si256((__m256i const * )(vsrc2 + i));
		const __m256i b1 = _mm256_loadu_si256((__m256i const * )(vdst1 + i));
		const __m256i b2 = _mm256_loadu_si256((__m256i const * )(vdst2 + i));

		const __m256i res1 = _mm256_xor_si256(a1,b1);
		const __m256i res2 = _mm256_xor_si256(a2,b2);
		assert(_mm256_testz_si256(res1, res2) == 1);
	}
}

static void __attribute__ ((noinline)) v3_CopyCompare(const int offsetIndex)
{
	const unsigned long long int pseudoIndex = offsetIndex * (1UL << 31);
	const char *vsrc1 = (const char *)src + pseudoIndex;
	const char *vsrc2 = (const char *)src + pseudoIndex + 32;
	const char *vdst1 = (const char *)dst + pseudoIndex;
	const char *vdst2 = (const char *)dst + pseudoIndex + 32;

	for (unsigned long long int i = 0; i < (1UL << 31); i += 64)
	{
		const __m256i a1 = _mm256_lddqu_si256 ((__m256i const * )(vsrc1 + i));
		const __m256i a2 = _mm256_lddqu_si256 ((__m256i const * )(vsrc2 + i));
		_mm256_storeu_si256((__m256i *)(vdst1 + i), a1);
		_mm256_storeu_si256((__m256i *)(vdst2 + i), a2);
	}

	for (unsigned long long int i = 0; i < (1UL << 31); i += 64)
	{
		const __m256i a1 = _mm256_lddqu_si256 ((__m256i const * )(vsrc1 + i));
		const __m256i a2 = _mm256_lddqu_si256 ((__m256i const * )(vsrc2 + i));
		const __m256i b1 = _mm256_lddqu_si256 ((__m256i const * )(vdst1 + i));
		const __m256i b2 = _mm256_lddqu_si256 ((__m256i const * )(vdst2 + i));

		const __m256i res1 = _mm256_xor_si256(a1,b1);
		const __m256i res2 = _mm256_xor_si256(a2,b2);
		assert(_mm256_testz_si256(res1, res2) == 1);
	}
}

static void __attribute__ ((noinline)) v4_CopyCompare(const int offsetIndex)
{
	unsigned long long int i = 0;

	const unsigned long long int pseudoIndex = offsetIndex * (1UL << 31);
	const char *vsrc1 = (const char *)src + pseudoIndex; 
	const char *vsrc2 = (const char *)src + pseudoIndex + 32;
	const char *vsrc3 = (const char *)src + pseudoIndex + 64; 
	const char *vsrc4 = (const char *)src + pseudoIndex + 32;
	const char *vdst1 = (const char *)dst + pseudoIndex + 96;
	const char *vdst2 = (const char *)dst + pseudoIndex + 32;
	const char *vdst3 = (const char *)dst + pseudoIndex + 64;
	const char *vdst4 = (const char *)dst + pseudoIndex + 96;

	for (i = 0; i < (1UL << 31); i += 128)
	{
		const __m256i a1 = _mm256_lddqu_si256 ((__m256i const * )(vsrc1 + i));
		const __m256i a2 = _mm256_lddqu_si256 ((__m256i const * )(vsrc2 + i));
		const __m256i a3 = _mm256_lddqu_si256 ((__m256i const * )(vsrc3 + i));
		const __m256i a4 = _mm256_lddqu_si256 ((__m256i const * )(vsrc4 + i));

		_mm256_storeu_si256((__m256i *)(vdst1 + i), a1);
		_mm256_storeu_si256((__m256i *)(vdst2 + i), a2);
		_mm256_storeu_si256((__m256i *)(vdst3 + i), a3);
		_mm256_storeu_si256((__m256i *)(vdst4 + i), a4);
	}

	i = 0;

	for (i = 0; i < (1UL << 31); i += 128)
	{
		const __m256i a1 = _mm256_lddqu_si256 ((__m256i const * )(vsrc1 + i));
		const __m256i a2 = _mm256_lddqu_si256 ((__m256i const * )(vsrc2 + i));
		const __m256i a3 = _mm256_lddqu_si256 ((__m256i const * )(vsrc3 + i));
		const __m256i a4 = _mm256_lddqu_si256 ((__m256i const * )(vsrc4 + i));
		const __m256i b1 = _mm256_lddqu_si256 ((__m256i const * )(vdst1 + i));
		const __m256i b2 = _mm256_lddqu_si256 ((__m256i const * )(vdst2 + i));
		const __m256i b3 = _mm256_lddqu_si256 ((__m256i const * )(vdst3 + i));
		const __m256i b4 = _mm256_lddqu_si256 ((__m256i const * )(vdst4 + i));

		const __m256i res1 = _mm256_xor_si256(a1,b1);
		const __m256i res2 = _mm256_xor_si256(a2,b2);
		const __m256i res3 = _mm256_xor_si256(a3,b3);
		const __m256i res4 = _mm256_xor_si256(a4,b4);
		assert((_mm256_testz_si256(res1, res2) == 1) && ((_mm256_testz_si256(res3, res4)) == 1));
	}
}

static void __attribute__ ((noinline)) v5_CopyCompare(const int offsetIndex)
{
	unsigned long long int i = 0;
	__m256i a1, a2, b1, b2;
	const unsigned long long int pseudoIndex = offsetIndex * (1UL << 31);

	const char *vsrc1 = (const char *)src + pseudoIndex + i/64 * 64;
	const char *vsrc2 = (const char *)src + pseudoIndex + i/64 * 64 + 32;
	const char *vdst1 = (const char *)dst + pseudoIndex + i/64 * 64;
	const char *vdst2 = (const char *)dst + pseudoIndex + i/64 * 64 +32;

#if ALIGNAREA
	a1 = _mm256_stream_load_si256 ((__m256i const * )vsrc1);
	a2 = _mm256_stream_load_si256 ((__m256i const * )vsrc2);

	for (i = 64; i < (1UL << 31) - 64; i += 64)
	{
		vsrc1 = (const char *)src + pseudoIndex + i/64 * 64; 
		vsrc2 = (const char *)src + pseudoIndex + i/64 * 64 + 32;
 		
		_mm256_storeu_si256(vdst1, a1);
		_mm256_storeu_si256(vdst2, a2);

		a1 = _mm256_stream_load_si256 ((__m256i const * )vsrc1);
		a2 = _mm256_stream_load_si256 ((__m256i const * )vsrc2);

		vdst1 = (const char *)dst + pseudoIndex + i/64 * 64;
		vdst2 = (const char *)dst + pseudoIndex + i/64 * 64 + 32;
	}
	_mm256_storeu_si256(vdst1, a1);
	_mm256_storeu_si256(vdst2, a2);

	i = 0;
	vsrc1 = (const char *)src + pseudoIndex + i/64 * 64;
	vsrc2 = (const char *)src + pseudoIndex + i/64 * 64 + 32;
	vdst1 = (const char *)dst + pseudoIndex + i/64 * 64;
	vdst2 = (const char *)dst + pseudoIndex + i/64 * 64 +32;

	a1 = _mm256_stream_load_si256 ((__m256i const * )vsrc1);
	a2 = _mm256_stream_load_si256 ((__m256i const * )vsrc2);
	b1 = _mm256_stream_load_si256 ((__m256i const * )vdst1);
	b2 = _mm256_stream_load_si256 ((__m256i const * )vdst2);

	for (i = 64; i < (1UL << 31) - 64; i += 64)
	{
		const __m256i res1 = _mm256_xor_si256(a1,b1);
		const __m256i res2 = _mm256_xor_si256(a2,b2);
		assert(_mm256_testz_si256(res1, res2) == 1);

		vsrc1 = (const char *)src + pseudoIndex + i/64 * 64;
		vsrc2 = (const char *)src + pseudoIndex + i/64 * 64 + 32;
		vdst1 = (const char *)dst + pseudoIndex + i/64 * 64;
		vdst2 = (const char *)dst + pseudoIndex + i/64 * 64 +32;

		a1 = _mm256_stream_load_si256 ((__m256i const * )vsrc1);
		a2 = _mm256_stream_load_si256 ((__m256i const * )vsrc2);
		b1 = _mm256_stream_load_si256 ((__m256i const * )vdst1);
		b2 = _mm256_stream_load_si256 ((__m256i const * )vdst2);
	}
	const __m256i res1 = _mm256_xor_si256(a1,b1);
	const __m256i res2 = _mm256_xor_si256(a2,b2);
	assert(_mm256_testz_si256(res1, res2) == 1);
#endif
}

static void __attribute__ ((noinline)) v6_CopyCompare(const int offsetIndex)
{
	unsigned long long int i = 0, j = 0;
	__m256i a1, a2, a3, a4, b1, b2, b3, b4;

	const unsigned long long int pseudoIndex = offsetIndex * (1UL << 31);

	const char *vsrc1 = (const char *)src + pseudoIndex; 
	const char *vsrc2 = (const char *)src + pseudoIndex + 32;
	const char *vsrc3 = (const char *)src + pseudoIndex + 64; 
	const char *vsrc4 = (const char *)src + pseudoIndex + 32;
	const char *vdst1 = (const char *)dst + pseudoIndex + 96;
	const char *vdst2 = (const char *)dst + pseudoIndex + 32;
	const char *vdst3 = (const char *)dst + pseudoIndex + 64;
	const char *vdst4 = (const char *)dst + pseudoIndex + 96;

	a1 = _mm256_lddqu_si256 ((__m256i const * )vsrc1);
	a2 = _mm256_lddqu_si256 ((__m256i const * )vsrc2);
	a3 = _mm256_lddqu_si256 ((__m256i const * )vsrc3);
	a4 = _mm256_lddqu_si256 ((__m256i const * )vsrc4);
	for (i = 128, j = 0; i < (1UL << 31) - 128; i += 128, j += 128)
	{
		_mm256_storeu_si256((__m256i *)(vdst1 + j), a1);
		_mm256_storeu_si256((__m256i *)(vdst2 + j), a2);
		_mm256_storeu_si256((__m256i *)(vdst3 + j), a3);
		_mm256_storeu_si256((__m256i *)(vdst4 + j), a4);

		b1 = _mm256_lddqu_si256 ((__m256i const * )(vdst1 + j));
		b2 = _mm256_lddqu_si256 ((__m256i const * )(vdst2 + j));
		b3 = _mm256_lddqu_si256 ((__m256i const * )(vdst3 + j));
		b4 = _mm256_lddqu_si256 ((__m256i const * )(vdst4 + j));

		const __m256i res1 = _mm256_xor_si256(a1,b1);
		const __m256i res2 = _mm256_xor_si256(a2,b2);
		const __m256i res3 = _mm256_xor_si256(a3,b3);
		const __m256i res4 = _mm256_xor_si256(a4,b4);

		a1 = _mm256_lddqu_si256 ((__m256i const * )(vsrc1 + i));
		a2 = _mm256_lddqu_si256 ((__m256i const * )(vsrc2 + i));
		a3 = _mm256_lddqu_si256 ((__m256i const * )(vsrc3 + i));
		a4 = _mm256_lddqu_si256 ((__m256i const * )(vsrc4 + i));

		assert((_mm256_testz_si256(res1, res2) & _mm256_testz_si256(res3, res4)) == 1);
	}
	_mm256_storeu_si256((__m256i *)(vdst1 + i), a1);
	_mm256_storeu_si256((__m256i *)(vdst2 + i), a2);
	_mm256_storeu_si256((__m256i *)(vdst3 + i), a3);
	_mm256_storeu_si256((__m256i *)(vdst4 + i), a4);

	b1 = _mm256_lddqu_si256 ((__m256i const * )(vdst1 + i));
	b2 = _mm256_lddqu_si256 ((__m256i const * )(vdst2 + i));
	b3 = _mm256_lddqu_si256 ((__m256i const * )(vdst3 + i));
	b4 = _mm256_lddqu_si256 ((__m256i const * )(vdst4 + i));

	const __m256i res1 = _mm256_xor_si256(a1,b1);
	const __m256i res2 = _mm256_xor_si256(a2,b2);
	const __m256i res3 = _mm256_xor_si256(a3,b3);
	const __m256i res4 = _mm256_xor_si256(a4,b4);

	assert((_mm256_testz_si256(res1, res2) & _mm256_testz_si256(res3, res4)) == 1);
}

static void __attribute__ ((noinline)) v7_CopyCompare(const int offsetIndex)
{
	unsigned long long int i = 0, j = 0;
	__m256i a1, a2, a3, a4, b1, b2, b3, b4;

	const unsigned long long int pseudoIndex = offsetIndex * (1UL << 31);
	const char *vsrc1 = (const char *)src + pseudoIndex; 
	const char *vsrc2 = (const char *)src + pseudoIndex + 32;
	const char *vsrc3 = (const char *)src + pseudoIndex + 64; 
	const char *vsrc4 = (const char *)src + pseudoIndex + 32;
	const char *vdst1 = (const char *)dst + pseudoIndex + 96;
	const char *vdst2 = (const char *)dst + pseudoIndex + 32;
	const char *vdst3 = (const char *)dst + pseudoIndex + 64;
	const char *vdst4 = (const char *)dst + pseudoIndex + 96;

	a1 = _mm256_lddqu_si256 ((__m256i const * )vsrc1);
	a2 = _mm256_lddqu_si256 ((__m256i const * )vsrc2);
	a3 = _mm256_lddqu_si256 ((__m256i const * )vsrc3);
	a4 = _mm256_lddqu_si256 ((__m256i const * )vsrc4);
	for (i = 128, j = 0; i < (1UL << 31) - 128; i += 128, j+= 128)
	{
		_mm_prefetch((vsrc1 + i), _MM_HINT_T0);
		_mm_prefetch((vsrc3 + i), _MM_HINT_T0);
		_mm_prefetch((vdst1 + i), _MM_HINT_T0);
		_mm_prefetch((vdst3 + i), _MM_HINT_T0);

		_mm256_storeu_si256((__m256i *)(vdst1 + j), a1);
		_mm256_storeu_si256((__m256i *)(vdst2 + j), a2);
		_mm256_storeu_si256((__m256i *)(vdst3 + j), a3);
		_mm256_storeu_si256((__m256i *)(vdst4 + j), a4);

		b1 = _mm256_lddqu_si256 ((__m256i const * )(vdst1 + j));
		b2 = _mm256_lddqu_si256 ((__m256i const * )(vdst2 + j));
		b3 = _mm256_lddqu_si256 ((__m256i const * )(vdst3 + j));
		b4 = _mm256_lddqu_si256 ((__m256i const * )(vdst4 + j));

		const __m256i res1 = _mm256_xor_si256(a1,b1);
		const __m256i res2 = _mm256_xor_si256(a2,b2);
		const __m256i res3 = _mm256_xor_si256(a3,b3);
		const __m256i res4 = _mm256_xor_si256(a4,b4);

		a1 = _mm256_lddqu_si256 ((__m256i const * )(vsrc1 + i));
		a2 = _mm256_lddqu_si256 ((__m256i const * )(vsrc2 + i));
		a3 = _mm256_lddqu_si256 ((__m256i const * )(vsrc3 + i));
		a4 = _mm256_lddqu_si256 ((__m256i const * )(vsrc4 + i));

		assert((_mm256_testz_si256(res1, res2) & _mm256_testz_si256(res3, res4)) == 1);
	}
	_mm256_storeu_si256((__m256i *)(vdst1 + i), a1);
	_mm256_storeu_si256((__m256i *)(vdst2 + i), a2);
	_mm256_storeu_si256((__m256i *)(vdst3 + i), a3);
	_mm256_storeu_si256((__m256i *)(vdst4 + i), a4);

	b1 = _mm256_lddqu_si256 ((__m256i const * )(vdst1 + i));
	b2 = _mm256_lddqu_si256 ((__m256i const * )(vdst2 + i));
	b3 = _mm256_lddqu_si256 ((__m256i const * )(vdst3 + i));
	b4 = _mm256_lddqu_si256 ((__m256i const * )(vdst4 + i));

	const __m256i res1 = _mm256_xor_si256(a1,b1);
	const __m256i res2 = _mm256_xor_si256(a2,b2);
	const __m256i res3 = _mm256_xor_si256(a3,b3);
	const __m256i res4 = _mm256_xor_si256(a4,b4);

	assert((_mm256_testz_si256(res1, res2) & _mm256_testz_si256(res3, res4)) == 1);
}

static void __attribute__ ((noinline)) v8_CopyCompare(const int offsetIndex)
{
        unsigned long long int i = 0, j = 0;
        __m256i a01, a02, a03, a04, b01, b02, b03, b04;
        __m256i a11, a12, a13, a14, b11, b12, b13, b14;
        __m256i res01, res02, res03, res04, res11, res12, res13,  res14;

        const unsigned long long int pseudoIndex = offsetIndex * (1UL << 31);

        const uint64_t vsrc[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)src + pseudoIndex +  0),
                (const uint64_t)((const char *)src + pseudoIndex + 32),
                (const uint64_t)((const char *)src + pseudoIndex + 64),
                (const uint64_t)((const char *)src + pseudoIndex + 96) };
        const uint64_t vsrc1[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)src + pseudoIndex + 128),
                (const uint64_t)((const char *)src + pseudoIndex + 160),
                (const uint64_t)((const char *)src + pseudoIndex + 192),
                (const uint64_t)((const char *)src + pseudoIndex + 224) };

        const uint64_t vdst[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)dst + pseudoIndex +  0),
                (const uint64_t)((const char *)dst + pseudoIndex + 32),
                (const uint64_t)((const char *)dst + pseudoIndex + 64),
                (const uint64_t)((const char *)dst + pseudoIndex + 96) };
        const uint64_t vdst1[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)dst + pseudoIndex + 128),
                (const uint64_t)((const char *)dst + pseudoIndex + 160),
                (const uint64_t)((const char *)dst + pseudoIndex + 192),
                (const uint64_t)((const char *)dst + pseudoIndex + 224) };

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
	unsigned int tickl, tickh;
	unsigned long long start, stop;

	int threadIndex = *(int *)userData;
	
	fprintf(stdout," Enter do_CopyCompare for %d!\n", threadIndex);
	do {
		/* v1 */
		if (__sync_fetch_and_and(&quit, 2) == 2) {
			fprintf(stdout," Sginal Exit do_CopyCompare!\n");
			pthread_exit(NULL);
			return NULL;
		}

		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        start = ((unsigned long long)tickh << 32)|tickl;
		v1_CopyCompare(threadIndex);
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        stop = ((unsigned long long)tickh << 32)|tickl;
		tickCount[threadIndex][0] = stop - start;

		/* v2 */
		if (__sync_fetch_and_and(&quit, 2) == 2) {
			fprintf(stdout," Sginal Exit do_CopyCompare!\n");
			pthread_exit(NULL);
			return NULL;
		}
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        start = ((unsigned long long)tickh << 32)|tickl;
		v2_CopyCompare(threadIndex);
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        stop = ((unsigned long long)tickh << 32)|tickl;
		tickCount[threadIndex][1] = stop - start;

		/* v3 */
		if (__sync_fetch_and_and(&quit, 2) == 2) {
			fprintf(stdout," Sginal Exit do_CopyCompare!\n");
			pthread_exit(NULL);
			return NULL;
		}
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        start = ((unsigned long long)tickh << 32)|tickl;
		v3_CopyCompare(threadIndex);
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        stop = ((unsigned long long)tickh << 32)|tickl;
		tickCount[threadIndex][2] = stop - start;

		/* v4 */
		if (__sync_fetch_and_and(&quit, 2) == 2) {
			fprintf(stdout," Sginal Exit do_CopyCompare!\n");
			pthread_exit(NULL);
			return NULL;
		}
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        start = ((unsigned long long)tickh << 32)|tickl;
		v4_CopyCompare(threadIndex);
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        stop = ((unsigned long long)tickh << 32)|tickl;
		tickCount[threadIndex][3] = stop - start;

		/* v5 */
		if (__sync_fetch_and_and(&quit, 2) == 2) {
			fprintf(stdout," Sginal Exit do_CopyCompare!\n");
			pthread_exit(NULL);
			return NULL;
		}
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        start = ((unsigned long long)tickh << 32)|tickl;
		/* if memory is not aligned leads to segfault ??? */
		v5_CopyCompare(threadIndex);
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        stop = ((unsigned long long)tickh << 32)|tickl;
		tickCount[threadIndex][4] = stop - start;

		/* v6 */
		if (__sync_fetch_and_and(&quit, 2) == 2) {
			fprintf(stdout," Sginal Exit do_CopyCompare!\n");
			pthread_exit(NULL);
			return NULL;
		}
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        start = ((unsigned long long)tickh << 32)|tickl;
		v6_CopyCompare(threadIndex);
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        stop = ((unsigned long long)tickh << 32)|tickl;
		tickCount[threadIndex][5] = stop - start;

		/* v7 */
		if (__sync_fetch_and_and(&quit, 2) == 2) {
			fprintf(stdout," Sginal Exit do_CopyCompare!\n");
			pthread_exit(NULL);
			return NULL;
		}
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        start = ((unsigned long long)tickh << 32)|tickl;
		v7_CopyCompare(threadIndex);
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        stop = ((unsigned long long)tickh << 32)|tickl;
		tickCount[threadIndex][6] = stop - start;
		
		/* v8 */
		if (__sync_fetch_and_and(&quit, 2) == 2) {
			fprintf(stdout," Sginal Exit do_CopyCompare!\n");
			pthread_exit(NULL);
			return NULL;
		}
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        start = ((unsigned long long)tickh << 32)|tickl;
		v8_CopyCompare(threadIndex);
		__asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
	        stop = ((unsigned long long)tickh << 32)|tickl;
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

	/* allocate copy & compare */
	for (int k = 0; k < numThreads; k++) {
		int temp = k;
		assert(pthread_create(&thread[k], NULL, do_CopyCompare, (void*)&temp) == 0);
		usleep(100);
	}
	__sync_fetch_and_or(&quit, 1);

	for (int k = 0; k < numThreads; k++)
		pthread_join(thread[k], NULL);

	/* display stats */
	printf("\033[2J");
        printf("\033[1;1H ---- Summary in ticks -----");
	int i = 0;
	long long unsigned total[7] = {0, 0, 0, 0, 0, 0, 0};
        for (; i < numThreads; i++) {
                printf("\033[%d;1H Thread-%03d: v1 (%10llu), v2 (%10llu), v3 (%10llu), v4 (%10llu), v5 (%10llu), v6(%10llu), v7(%10llu), v8(%10llu)",
                        i + 2, i, tickCount[i][0], tickCount[i][1], tickCount[i][2], tickCount[i][3], tickCount[i][4], tickCount[i][5], tickCount[i][6], tickCount[i][7]);
		total[0] += tickCount[i][0];
		total[1] += tickCount[i][1];
		total[2] += tickCount[i][2];
		total[3] += tickCount[i][3];
		total[4] += tickCount[i][4];
		total[5] += tickCount[i][5];
		total[6] += tickCount[i][6];
		total[7] += tickCount[i][7];
	}
        printf("\033[%d;1H --- with Pstate ticks can be unreliable ---", i + 2);
	printf("\033[%d;1H Total: %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu",
		i + 3, total[0], total[1], total[2], total[3], total[4], total[5], total[6], total[7]);
        printf("\033[%d;1H speedup %%: %f, %f, %f, %f, %f, %f, %f, %f",
		i + 4,
		((float)(total[0] - 0) * 100)/ (float)total[0],
		((float)(total[0] - total[1]) * 100)/ (float)total[1],
		((float)(total[0] - total[2]) * 100)/ (float)total[2],
		((float)(total[0] - total[3]) * 100)/ (float)total[3],
		((float)(total[0] - total[4]) * 100)/ (float)total[4],
		((float)(total[0] - total[5]) * 100)/ (float)total[5],
		((float)(total[0] - total[6]) * 100)/ (float)total[6],
		((float)(total[0] - total[7]) * 100)/ (float)total[7] );
        printf("\033[%d;1H ---------------------------------", i + 5);
	
        printf("\033[%d;1H", i + 5);	

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
