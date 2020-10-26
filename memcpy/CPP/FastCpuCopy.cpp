#include <immintrin.h>
#include <x86intrin.h>

#include "FastCpuCopy.h"

static void __attribute__ ((noinline)) FastCopyCompare(void *userData)
{
	MultiMemcpy::Worker *ptr = (MultiMemcpy::Worker *)userData;
	const char * src = (const char *) ptr->src;
	const char * dst = (const char *) ptr->dst;
	const size_t len = ptr->size;

        unsigned long long int i = 0;
        __m256i a01, a02, a03, a04, b01, b02, b03, b04;
        __m256i a11, a12, a13, a14, b11, b12, b13, b14;

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
        __m256i vector_offset = _mm256_set_epi64x(256, 256, 256, 256);

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
                /* perform memcpy */
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

                _mm_prefetch(((const char*)sptr[0] + 256), _MM_HINT_NTA);
                _mm_prefetch(((const char*)dptr[0] + 256), _MM_HINT_NTA);

                vector_vsrc = _mm256_add_epi64 (vector_vsrc, vector_offset);
                vector_vsrc1 = _mm256_add_epi64 (vector_vsrc, vector_offset);
                vector_vdst = _mm256_add_epi64 (vector_vdst, vector_offset);
                vector_vdst1 = _mm256_add_epi64 (vector_vdst, vector_offset);
                sptr = (const uint64_t *)&vector_vsrc;
                sptr1 = (const uint64_t *)&vector_vsrc1;
                dptr = (const uint64_t *)&vector_vdst;
                dptr1 = (const uint64_t *)&vector_vdst1;

                /* perform memcmp */
                /*const*/register  __m256i res01 = _mm256_xor_si256(a01,b01);
                /*const*/register  __m256i res02 = _mm256_xor_si256(a02,b02);
                /*const*/register  __m256i res03 = _mm256_xor_si256(a03,b03);
                /*const*/register  __m256i res04 = _mm256_xor_si256(a04,b04);
                /*const*/register  __m256i res11 = _mm256_xor_si256(a11,b11);
                /*const*/register  __m256i res12 = _mm256_xor_si256(a12,b12);
                /*const*/register  __m256i res13 = _mm256_xor_si256(a13,b13);
                /*const*/register  __m256i res14 = _mm256_xor_si256(a14,b14);

                assert((_mm256_testz_si256(res01, res02) & _mm256_testz_si256(res03, res04)) == 1);
                assert((_mm256_testz_si256(res11, res12) & _mm256_testz_si256(res13, res14)) == 1);
        }

        _mm256_zeroupper();
}

MultiMemcpy::MultiMemcpy()
{
  for (int i = 0; i < MULTIMEMCPY_THREADS; ++i)
  {
	m_workers[i].id = (1 << i);
	m_workers[i].state = 1;
  }
}

MultiMemcpy::~MultiMemcpy()
{
  for(int i = 0; i < MULTIMEMCPY_THREADS; ++i)
  {
	m_workers[i].state = 0;
  }
}

void MultiMemcpy::Copy(void * dst, void * src, size_t size)
{
	/* prepare the src, dst and block size of each thread */
	const size_t block = (size / MULTIMEMCPY_THREADS);
	for (int i = 0; i < MULTIMEMCPY_THREADS; ++i)
	{
		m_workers[i].dst  = (uint8_t *)dst + i * block;
		m_workers[i].src  = (uint8_t *)src + i * block;

		m_workers[i].size = block;
		if (i == MULTIMEMCPY_THREADS - 1)
			m_workers[i].size = size - (block * i);

		std::cout << "size: " << m_workers[i].size << "\n";
  	}

	/* Launch MULTIMEMCPY_THREADS with specific param */
	std::thread threads[MULTIMEMCPY_THREADS];
	for (int i = 0; i < MULTIMEMCPY_THREADS; ++i)
	{
		threads[i] = std::thread(&FastCopyCompare, &m_workers[i]);
	}

	for (int i = 0; i < MULTIMEMCPY_THREADS; ++i)
	{
		threads[i].join();
	}
}

