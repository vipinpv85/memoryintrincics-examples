#include "runlogic.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DISPLAYSTATS
static long long unsigned total[NUMVER];
#endif

/*
 * Function	: v1_CopyCompare
 *
 * Purpose	: Per thread operation to performs memcpy and memcmp sequentially using glibc function
 *
 * INPUT: src, dst, len
 * 	src: char pointer to start of source address
 *   	dst: char pointer to start of destination address
 *   	len: length to copy and compare between src and dst
 *
 * OUTPUT: none
 *
 * RETURN: void
 *
 * NOTE: 
 */
#ifdef __cplusplus
static void __attribute__ ((noinline)) v1_CopyCompare(const char * src, const char * dst, const size_t len)
#else
static void __attribute__ ((noinline)) v1_CopyCompare(const char * restrict src, const char * restrict dst, const size_t len)
#endif
{
	void *ptr = memcpy((void *)dst, (void *)src, len);

	assert(ptr == dst); /* to valdiate if copy has been completed */
	assert(memcmp(dst, src, len) == 0);
}

/*
 * Function	: v2_CopyCompare
 *
 * Purpose	: Per thread operation to performs memcpy and memcmp sequentially using SIMD intrinsics (AVX2)
 *
 * INPUT: src, dst, len
 * 	src: char pointer to start of source address
 *   	dst: char pointer to start of destination address
 *   	len: length to copy and compare between src and dst
 *
 * OUTPUT: none
 *
 * RETURN: void
 *
 * NOTE:
 *	a. mimics behaviour or glibc memcpy and memcp sequentially. 
 *	b. to maximize use of YMM registers, multiple variable declaration is used.
 *  	c. to minimize use of indivual address update, SIMD based function pointers are executed.
 *	d. performance speed up is 7.17 of v1_CopyCompare
 */
#ifdef __cplusplus
static void __attribute__ ((noinline)) v2_CopyCompare(const char * src, const char * dst, const size_t len)
#else
static void __attribute__ ((noinline)) v2_CopyCompare(const char * restrict src, const char * restrict dst, const size_t len)
#endif
{
	unsigned long long int i = 0;
	__m256i a01, a02, a03, a04, b01, b02, b03, b04;
	__m256i a11, a12, a13, a14, b11, b12, b13, b14;
	const uint64_t *sptr, *sptr1, *dptr, *dptr1;

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
	sptr = (const uint64_t *)&vector_vsrc;
	sptr1 = (const uint64_t *)&vector_vsrc1;
	dptr = (const uint64_t *)&vector_vdst;
	dptr1 = (const uint64_t *)&vector_vdst1;

	/* perform memcpy */
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

		_mm_prefetch((const char*)(sptr[0] + 256), _MM_HINT_NTA);
		_mm_prefetch((const char*)(dptr[0] + 256), _MM_HINT_NTA);

		vector_vsrc = _mm256_add_epi64 (vector_vsrc, vector_offset);
		vector_vsrc1 = _mm256_add_epi64 (vector_vsrc, vector_offset);
		vector_vdst = _mm256_add_epi64 (vector_vdst, vector_offset);
		vector_vdst1 = _mm256_add_epi64 (vector_vdst, vector_offset);

		sptr = (const uint64_t *)&vector_vsrc;
		sptr1 = (const uint64_t *)&vector_vsrc1;
		dptr = (const uint64_t *)&vector_vdst;
		dptr1 = (const uint64_t *)&vector_vdst1;
	}

	vector_vsrc = _mm256_lddqu_si256((__m256i const *)vsrc);
	vector_vsrc1 = _mm256_lddqu_si256((__m256i const *)vsrc1);
	vector_vdst = _mm256_lddqu_si256((__m256i const *)vdst);
	vector_vdst1 = _mm256_lddqu_si256((__m256i const *)vdst1);
	vector_offset = _mm256_set_epi64x(128, 128, 128, 128);
	sptr = (const uint64_t *)&vector_vsrc;
	sptr1 = (const uint64_t *)&vector_vsrc1;
	dptr = (const uint64_t *)&vector_vdst;
	dptr1 = (const uint64_t *)&vector_vdst1;

	/* perform memcmp */
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

		_mm_prefetch((const char*)(sptr[0] + 256), _MM_HINT_NTA);
		_mm_prefetch((const char*)(dptr[0] + 256), _MM_HINT_NTA);

		vector_vsrc = _mm256_add_epi64 (vector_vsrc, vector_offset);
		vector_vsrc1 = _mm256_add_epi64 (vector_vsrc, vector_offset);
		vector_vdst = _mm256_add_epi64 (vector_vdst, vector_offset);
		vector_vdst1 = _mm256_add_epi64 (vector_vdst, vector_offset);
		sptr = (const uint64_t *)&vector_vsrc;
		sptr1 = (const uint64_t *)&vector_vsrc1;
		dptr = (const uint64_t *)&vector_vdst;
		dptr1 = (const uint64_t *)&vector_vdst1;

		__m256i res01 = _mm256_xor_si256(a01,b01);
		__m256i res02 = _mm256_xor_si256(a02,b02);
		__m256i res03 = _mm256_xor_si256(a03,b03);
		__m256i res04 = _mm256_xor_si256(a04,b04);
		__m256i res11 = _mm256_xor_si256(a11,b11);
		__m256i res12 = _mm256_xor_si256(a12,b12);
		__m256i res13 = _mm256_xor_si256(a13,b13);
		__m256i res14 = _mm256_xor_si256(a14,b14);

		uint64_t reg1 = _mm256_testz_si256(res01, res02);
		uint64_t reg2 = _mm256_testz_si256(res03, res04);
		uint64_t reg3 = _mm256_testz_si256(res11, res12);
		uint64_t reg4 = _mm256_testz_si256(res13, res14);

		assert ((reg1 == 1) && (reg2 == 1) && (reg3 == 1) && (reg4 == 1));
	}

	_mm256_zeroupper();
}

/*
 * Function	: v3_CopyCompare
 *
 * Purpose	: Per thread operation to performs memcpy and memcmp in same for loop using Agner vector class version2.0
 *
 * INPUT: src, dst, len
 * 	src: char pointer to start of source address
 *   	dst: char pointer to start of destination address
 *   	len: length to copy and compare between src and dst
 *
 * OUTPUT: none
 *
 * RETURN: void
 *
 * NOTE:
 *	a. memcpy and memcp are interleave in the same loop, to reduce loop iteration and improve the cache locality of src data. 
 *	b. to maximize use of YMM registers, multiple variable declaration is used.
 *  	c. to minimize use of indivual address update, SIMD based function pointers are executed.
 *	d. performance speed up is 15.42 of v1_CopyCompare
 */
#ifdef __cplusplus
static void __attribute__ ((noinline)) v3_CopyCompare(const char * src, const char * dst, const size_t len)
#else
static void __attribute__ ((noinline)) v3_CopyCompare(const char * restrict src, const char * restrict dst, const size_t len)
#endif
{
        unsigned long long int i = 0;
        unsigned long int offset[4] = {256, 256, 256, 256};

        const uint64_t *sptr1, *sptr2, *dptr1, *dptr2;

        Vec32uc a0, a1, a2, a3, a4, a5, a6, a7;
        Vec32uc b0, b1, b2, b3, b4, b5, b6, b7;
        Vec4uq vector_vsrc1, vector_vsrc2;
        Vec4uq vector_vdst1, vector_vdst2;
        Vec4uq vector_offset;

        const uint64_t vsrc1[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)src +   0),
                (const uint64_t)((const char *)src +  32),
                (const uint64_t)((const char *)src +  64),
                (const uint64_t)((const char *)src +  96) };
        const uint64_t vsrc2[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)src + 128),
                (const uint64_t)((const char *)src + 160),
                (const uint64_t)((const char *)src + 192),
                (const uint64_t)((const char *)src + 224) };

        const uint64_t vdst1[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)dst +  0),
                (const uint64_t)((const char *)dst + 32),
                (const uint64_t)((const char *)dst + 64),
                (const uint64_t)((const char *)dst + 96) };
        const uint64_t vdst2[4]  __attribute__ ((aligned (16)))= {
                (const uint64_t)((const char *)dst + 128),
                (const uint64_t)((const char *)dst + 160),
                (const uint64_t)((const char *)dst + 192),
                (const uint64_t)((const char *)dst + 224) };
        /* address stored in vector format */
        vector_vsrc1.load((__m256i const *)vsrc1);
        vector_vsrc2.load((__m256i const *)vsrc2);
        vector_vdst1.load((__m256i const *)vdst1);
        vector_vdst2.load((__m256i const *)vdst2);
        vector_offset.load(&offset);

        /* local pointer to access 32 byte content */
        sptr1 = (const uint64_t *)&vector_vsrc1;
        sptr2 = (const uint64_t *)&vector_vsrc2;
        dptr1 = (const uint64_t *)&vector_vdst1;
        dptr2 = (const uint64_t *)&vector_vdst2;

        /* perform memcpy */
        for (i = 0; i < len; i += 256) {
                a0.load((const unsigned char *)sptr1[0]);
                a1.load((const unsigned char *)sptr1[1]);
                a2.load((const unsigned char *)sptr1[2]);
                a3.load((const unsigned char *)sptr1[3]);
                a4.load((const unsigned char *)sptr2[0]);
                a5.load((const unsigned char *)sptr2[1]);
                a6.load((const unsigned char *)sptr2[2]);
                a7.load((const unsigned char *)sptr2[3]);

                _mm_prefetch((const char*)(sptr1[0] + 256), _MM_HINT_NTA);

                a0.store((void *)dptr1[0]);
                a1.store((void *)dptr1[1]);
                a2.store((void *)dptr1[2]);
                a3.store((void *)dptr1[3]);
                a4.store((void *)dptr2[0]);
                a5.store((void *)dptr2[1]);
                a6.store((void *)dptr2[2]);
                a7.store((void *)dptr2[3]);

                vector_vsrc1 += vector_offset;
                vector_vsrc2 += vector_offset;
                vector_vdst1 += vector_offset;
                vector_vdst2 += vector_offset;

                sptr1 = (const uint64_t *)&vector_vsrc1;
                sptr2 = (const uint64_t *)&vector_vsrc2;
                dptr1 = (const uint64_t *)&vector_vdst1;
                dptr2 = (const uint64_t *)&vector_vdst2;
        }
        /* address stored in vector format */
        vector_vsrc1.load((__m256i const *)vsrc1);
        vector_vsrc2.load((__m256i const *)vsrc2);
        vector_vdst1.load((__m256i const *)vdst1);
        vector_vdst2.load((__m256i const *)vdst2);
        vector_offset.load(&offset);

        /* local pointer to access 32 byte content */
        sptr1 = (const uint64_t *)&vector_vsrc1;
        sptr2 = (const uint64_t *)&vector_vsrc2;
        dptr1 = (const uint64_t *)&vector_vdst1;
        dptr2 = (const uint64_t *)&vector_vdst2;

        /* perform memcmp */
        for (i = 0; i < len; i += 256) {
                a0.load((const unsigned char *)sptr1[0]);
                a1.load((const unsigned char *)sptr1[1]);
                a2.load((const unsigned char *)sptr1[2]);
                a3.load((const unsigned char *)sptr1[3]);
                a4.load((const unsigned char *)sptr2[0]);
                a5.load((const unsigned char *)sptr2[1]);
                a6.load((const unsigned char *)sptr2[2]);
                a7.load((const unsigned char *)sptr2[3]);
                b0.load((const unsigned char *)dptr1[0]);
                b1.load((const unsigned char *)dptr1[1]);
                b2.load((const unsigned char *)dptr1[2]);
                b3.load((const unsigned char *)dptr1[3]);
                b4.load((const unsigned char *)dptr2[0]);
                b5.load((const unsigned char *)dptr2[1]);
                b6.load((const unsigned char *)dptr2[2]);
                b7.load((const unsigned char *)dptr2[3]);

                _mm_prefetch(((const char*)sptr1[0] + 256), _MM_HINT_NTA);
                _mm_prefetch(((const char*)dptr1[0] + 256), _MM_HINT_NTA);

                vector_vsrc1 += vector_offset;
                vector_vsrc2 += vector_offset;
                vector_vdst1 += vector_offset;
                vector_vdst2 += vector_offset;

                sptr1 = (const uint64_t *)&vector_vsrc1;
                sptr2 = (const uint64_t *)&vector_vsrc2;
                dptr1 = (const uint64_t *)&vector_vdst1;
                dptr2 = (const uint64_t *)&vector_vdst2;

                Vec32cb res0 = (a0 == b0);
                Vec32cb res1 = (a1 == b1);
                Vec32cb res2 = (a2 == b2);
                Vec32cb res3 = (a3 == b3);
                Vec32cb res4 = (a4 == b4);
                Vec32cb res5 = (a5 == b5);
                Vec32cb res6 = (a6 == b6);
                Vec32cb res7 = (a7 == b7);

                assert(horizontal_or(res0) == 1);
                assert(horizontal_or(res1) == 1);
                assert(horizontal_or(res2) == 1);
                assert(horizontal_or(res3) == 1);
                assert(horizontal_or(res4) == 1);
                assert(horizontal_or(res5) == 1);
                assert(horizontal_or(res6) == 1);
                assert(horizontal_or(res7) == 1);
        }

	_mm256_zeroupper();
}

/*
 * Function	: v8_CopyCompare
 *
 * Purpose	: Per thread operation to performs memcpy and memcmp in same for loop using SIMD intrinsics (AVX2)
 *
 * INPUT: src, dst, len
 * 	src: char pointer to start of source address
 *   	dst: char pointer to start of destination address
 *   	len: length to copy and compare between src and dst
 *
 * OUTPUT: none
 *
 * RETURN: void
 *
 * NOTE:
 *	a. memcpy and memcp are interleave in the same loop, to reduce loop iteration and improve the cache locality of src data. 
 *	b. to maximize use of YMM registers, multiple variable declaration is used.
 *  	c. to minimize use of indivual address update, SIMD based function pointers are executed.
 *	d. performance speed up is 15.42 of v1_CopyCompare
 */
#ifdef __cplusplus
static void __attribute__ ((noinline)) v8_CopyCompare(const char * src, const char * dst, const size_t len)
#else
static void __attribute__ ((noinline)) v8_CopyCompare(const char * restrict src, const char * restrict dst, const size_t len)
#endif
{
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

/*
 * Function	: thread_CopyCompare
 *
 * Purpose	: Per thread function wrapper to execute and collect ticks
 *
 * INPUT: 
 * 	src: char pointer to start of source address
 *   	dst: char pointer to start of destination address
 *   	len: length to copy and compare between src and dst
 *
 * OUTPUT: none
 *
 * RETURN: void
 *
 * NOTE:
 */
static void *thread_CopyCompare(void *userData)
{
	myThreadData_t *threadData = (myThreadData_t *)userData;
	uint64_t start, stop;
	unsigned int ignore;
	int threadIndex = ((myThreadData_t *)userData)->threadIndex;

	fprintf(stdout," Enter do_CopyCompare for %d!\n", threadIndex);
	do {
		/* Execute v1 */
		start = __rdtscp(&ignore);
		v1_CopyCompare(threadData->src, threadData->dst, threadData->len);
		stop = __rdtscp(&ignore);
		threadData->tickCount[0] += stop - start;

		/* Execute v2 */
		start = __rdtscp(&ignore);
		v2_CopyCompare(threadData->src, threadData->dst, threadData->len);
		stop = __rdtscp(&ignore);
		threadData->tickCount[1] += stop - start;

		/* Execute v3 */
		start = __rdtscp(&ignore);
		v3_CopyCompare(threadData->src, threadData->dst, threadData->len);
		stop = __rdtscp(&ignore);
		threadData->tickCount[2] += stop - start;

		/* Execute v8 */
		start = __rdtscp(&ignore);
		v8_CopyCompare(threadData->src, threadData->dst, threadData->len);
		stop = __rdtscp(&ignore);
		threadData->tickCount[7] += stop - start;
	} while(0);
	fprintf(stdout," Exit do_CopyCompare %d!\n", threadIndex);

	return (void *)threadData->dst;
}


void do_CopyCompare(const void *src, const void *dst, size_t len, uint8_t is_mmap)
{
	pthread_t thread[NUMTHREADS];
	assert(len == (1UL << 30) * NUMMEMGB);

	fprintf(stdout, "src %p dst %p len %lu\n", src, dst, len);
	myThreadData_t userData[8] __attribute__ ((aligned (16)));

	for (uint8_t i = 0; i < NUMTHREADS; i++) {
		userData[i].src = (const char *)src + (1UL << 31) * i;
		userData[i].dst = (const char *)dst + (1UL << 31) * i;
		userData[i].len = (1UL << 31);
		userData[i].threadIndex = i;
		for (uint8_t j = 0; j < NUMVER; j++)
			userData[i].tickCount[j] = 0;
	}

	/* allocate copy & compare */
	for (int k = 0; k < NUMTHREADS; k++) {
		assert(pthread_create(&thread[k], NULL, thread_CopyCompare, (void*)&userData[k]) == 0);
	}

	for (int k = 0; k < NUMTHREADS; k++)
		pthread_join(thread[k], NULL);

#if DISPLAYSTATS
	/* display stats */
	printf("\033[2J");
        printf("\033[1;1H ---- Summary in ticks -----");
	int i = 0;
        for (; i < NUMTHREADS; i++) {
                printf("\033[%d;1H Thread-%03d: v1 (%10llu), v2 (%10llu), v3 (%10llu), v8(%10llu)",
                        i + 2, i, userData[i].tickCount[0], userData[i].tickCount[1], userData[i].tickCount[2], userData[i].tickCount[7]);
		total[0] += userData[i].tickCount[0];
		total[1] += userData[i].tickCount[1];
		total[2] += userData[i].tickCount[2];
		total[7] += userData[i].tickCount[7];
	}
        printf("\033[%d;1H --- with Pstate ticks can be unreliable ---", i + 2);
	printf("\033[%d;1H Total: %llu, %llu, %llu, %llu with (%s)",
		i + 3, total[0], total[1], total[1], total[7], (is_mmap)?"mmap":"malloc");
        printf("\033[%d;1H speed-up %%: %f, %f, %f, %f",
		i + 4,
		((float)(total[0] - 0) * 100)/ (float)total[0],
		((float)(total[0] - total[1]) * 100)/ (float)total[1],
		((float)(total[0] - total[2]) * 100)/ (float)total[2],
		((float)(total[0] - total[7]) * 100)/ (float)total[7]
		);
        printf("\033[%d;1H ---------------------------------", i + 5);
	
        printf("\033[%d;1H", i + 5);
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
#endif
}

#ifdef __cplusplus
} //end extern "C"
#endif

