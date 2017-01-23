#include <immintrin.h> 
#include <inttypes.h>

#define CNTMASK ((__m256i) {0x8000000000000000, 0x8000000000000000, 0x8000000000000000, 0x8000000000000000})

#define ETHMETA1 ((uint64_t)0x86DD080681000800)
#define IPMETA1 ((uint64_t)0x1106040132000000)


//#define CMP_ETHTYPE_AVX2(a) _mm256_movemask_epi8(_mm256_cmpeq_epi16(_mm256_set_epi16( *a, *a, *a, *a, *(a+1), *(a+1), *(a+1), *(a+1), *(a+2), *(a+2), *(a+2), *(a+2), *(a+3), *(a+3), *(a+3), *(a+3)), (__m256i){ETHMETA1, ETHMETA1, ETHMETA1, ETHMETA1}))
#define CMP_ETHTYPE_AVX2(a) _mm256_movemask_epi8(_mm256_cmpeq_epi16((__m256i){*a, *(a+1), *(a+2), *(a+3)}, (__m256i){ETHMETA1, ETHMETA1, ETHMETA1, ETHMETA1}))

static void testmain_eth(const short *a, const uint64_t *counters) __attribute__((always_inline));
static void testmain_ip(const char *a, uint64_t *counters) __attribute__((always_inline));

static int  cmpEthType(const uint64_t *a) __attribute__((always_inline));
static int  cmpIpType(const uint64_t *a) __attribute__((always_inline));

static int  cmpEthType_avx2(const uint64_t *a) __attribute__((always_inline));
static int  cmpEthType_sse(const uint64_t *a) __attribute__((always_inline));

static int  cmpIPType_avx2(const uint64_t *a) __attribute__((always_inline));
static int  cmpIpType_sse(const uint64_t *a) __attribute__((always_inline));

static void counter_eth(uint64_t *counters, uint64_t *t2) __attribute__((always_inline));
static void counter_ip(uint64_t *counters, uint64_t *t2) __attribute__((always_inline));

static inline int naiveChecksum(const char * const target, size_t targetLength) __attribute__((always_inline));
static inline int avxChecksumV1(const char * const target, size_t targetLength) __attribute__((always_inline)); 
static inline int avxUdpChecksumV1(const char * const target, size_t targetLength) __attribute__((always_inline));
static inline int avxUdpChecksumV2(const char * const target, size_t targetLength) __attribute__((always_inline));
static inline int avxmmxUdpChecksumV1(const char * const target, size_t targetLength) __attribute__((always_inline));

__attribute__((always_inline)) inline int avxmmxChecksumV1(const char * const target, size_t targetLength)
{
        uint64_t checksum = 0;
        size_t offset = 0;
        __m256i vec, lVec, hVec, sum, zero = _mm256_setzero_si256();
        __m128i vec1, lVec1, hVec1, sum1, zero1 = _mm_setzero_si128 ();

        if(targetLength >= 32) {
                for(; offset <= targetLength - 32; offset += 32)
                {
                        vec = _mm256_loadu_si256((__m256i const *)(target + offset));
                        //vec = (__m256i) {*(const uint64_t *)(target + 0 + offset), *(const uint64_t *)(target + 8 + offset), *(const uint64_t *)(target + 16 + offset), *(const uint64_t *)(target + 24 + offset)};
                        sum = _mm256_add_epi32(_mm256_unpacklo_epi16(vec, zero),
                                               _mm256_unpackhi_epi16(vec, zero));

                        sum = _mm256_hadd_epi32(sum, sum);
                        sum = _mm256_hadd_epi32(sum, sum);

                        checksum += (uint64_t) _mm256_extract_epi32(sum, 0) + (uint64_t) _mm256_extract_epi32(sum, 7);
                }
                checksum = ((checksum & 0xffffffff00000000) >> 32) + (checksum & 0xffffffff);
        }

        if ((targetLength - offset) >= 16) {
                vec1 = _mm_loadu_si128 ((__m128i const*) (target + offset));
                sum1 = _mm_add_epi32 (_mm_unpacklo_epi16(vec1, zero1), _mm_unpackhi_epi16(vec1, zero1));

                sum1 = _mm_hadd_epi32(sum1, sum1);
                sum1 = _mm_hadd_epi32(sum1, sum1);

                //checksum += (uint64_t) _mm_extract_epi32(sum1, 0) + (uint64_t) _mm_extract_epi32(sum1, 3);
                checksum += (uint64_t) _mm_extract_epi32(sum1, 0);
                checksum = ((checksum & 0xffffffff00000000) >> 32) + (checksum & 0xffffffff);

                offset += 16;
        }

        if ((targetLength - offset) >= 2) {
                for(;(targetLength -offset) >= 2; offset+=2)
                {
                        checksum +=  (*(uint16_t *) ((uint8_t *) target + offset) );
                }
                offset -= 2;
        }

        if ((targetLength - offset) == 1) {
                checksum += *((uint8_t *) target + offset);
        }

        checksum = ((checksum & 0xffff0000) >> 16) + (checksum & 0xffff);
        checksum = ((checksum & 0xffff0000) >> 16) + (checksum & 0xffff);

        return checksum;
}


__attribute__((always_inline)) inline int avxUdpChecksumV2(const char * const target, size_t targetLength)
{
        unsigned int checksum = 0;
        size_t offset = 0;
        uint16_t *buff = NULL;
        __m256i vec, lVec, hVec, sum;

        if(targetLength >= 32) {
                for(; offset <= targetLength - 32; offset += 32) {
                        vec = _mm256_loadu_si256((__m256i const *)(target + offset));
                        lVec = _mm256_unpacklo_epi16(vec, (__m256i) {(uint64_t) 0x0, (uint64_t) 0x0, (uint64_t) 0x0, (uint64_t) 0x0});
                        hVec = _mm256_unpackhi_epi16(vec, (__m256i) {(uint64_t) 0x0, (uint64_t) 0x0, (uint64_t) 0x0, (uint64_t) 0x0});

                        sum = _mm256_add_epi32(lVec, hVec);

                        sum = _mm256_hadd_epi32(sum, sum);
                        sum = _mm256_hadd_epi32(sum, sum);
                        sum = _mm256_hadd_epi32(sum, sum);


                        checksum += _mm256_extract_epi16(sum, 0) + _mm256_extract_epi16(sum, 15);
                }
        }

        if (targetLength - offset >= 2) {
                for(;(targetLength -offset) >= 2; offset+=2)
                {
                        checksum +=  (*(uint16_t *) ((uint8_t *) target + offset) );
                }
                offset -= 2;
        }

        if (targetLength - offset)  {
                checksum += *((uint8_t *) target + offset);
        }

        checksum = ((checksum & 0xffff0000) >> 16) + (checksum & 0xffff);
        checksum = ((checksum & 0xffff0000) >> 16) + (checksum & 0xffff);

        return checksum;
}


__attribute__((always_inline)) inline int avxUdpChecksumV1(const char * const target, size_t targetLength)
{
        const __m256i zeroVec = _mm256_setzero_si256();
        unsigned int checksum = 0;
        size_t offset = 0;
        uint16_t *buff = NULL;

        if(targetLength >= 32) {
                for(; offset <= targetLength - 32; offset += 32) {
                        __m256i vec = _mm256_loadu_si256((__m256i const *)(target + offset));
                        __m256i lVec = _mm256_unpacklo_epi16(vec, zeroVec);
                        __m256i hVec = _mm256_unpackhi_epi16(vec, zeroVec);
                        __m256i sum = _mm256_add_epi32(lVec, hVec);
                        sum = _mm256_hadd_epi32(sum, sum);
                        sum = _mm256_hadd_epi32(sum, sum);
                        sum = _mm256_hadd_epi32(sum, sum);
                        checksum += _mm256_extract_epi16(sum, 0) +
                                _mm256_extract_epi16(sum, 15);
                }
        }

        if (targetLength - offset >= 2) {
                for(;(targetLength -offset) >= 2; offset+=2)
                {
                        checksum +=  (*(uint16_t *) ((uint8_t *) target + offset) );
                }
                offset -= 2;
        }

        if (targetLength - offset)  {
                checksum += *((uint8_t *) target + offset);
        }

        checksum = ((checksum & 0xffff0000) >> 16) + (checksum & 0xffff);
        checksum = ((checksum & 0xffff0000) >> 16) + (checksum & 0xffff);

        return checksum;
}


__attribute__((always_inline)) static inline int naiveChecksum(const char * const target, size_t targetLength) 
{
	unsigned int checksum = 0;
	for(size_t i = 0; i < targetLength; ++i) {
		checksum += (unsigned int) target[i];
	}
	return checksum % 256;
}

__attribute__((always_inline)) inline int avxChecksumV1(const char * const target, size_t targetLength) 
{
	const __m256i zeroVec = _mm256_setzero_si256();
	unsigned int checksum = 0;		
	size_t offset = 0;

	if(targetLength >= 32) {
		for(; offset <= targetLength - 32; offset += 32) {
			__m256i vec = _mm256_loadu_si256((__m256i const *)(target + offset));
			__m256i lVec = _mm256_unpacklo_epi8(vec, zeroVec);
			__m256i hVec = _mm256_unpackhi_epi8(vec, zeroVec);
			__m256i sum = _mm256_add_epi16(lVec, hVec);
			sum = _mm256_hadd_epi16(sum, sum);
			sum = _mm256_hadd_epi16(sum, sum);
			sum = _mm256_hadd_epi16(sum, sum);
			checksum += _mm256_extract_epi16(sum, 0) + 
				_mm256_extract_epi16(sum, 15);		
		}
	}

	for(; offset < targetLength; ++offset) {
		checksum += (unsigned int) target[offset];
	}

	return checksum % 256;
}

__attribute__((always_inline)) static inline int cmpIpType(const uint64_t *a)
{
    uint8_t data1 = (IPMETA1 & 0xff) >>  0;
    uint8_t data2 = (IPMETA1 & 0xff00) >> 8;
    uint8_t data3 = (IPMETA1 & 0xff0000) >>  16;
    uint8_t data4 = (IPMETA1 & 0xff000000) >>  24;
    uint8_t data5 = (IPMETA1 & 0xff00000000) >>  32;
    uint8_t data6 = (IPMETA1 & 0xff0000000000) >> 40;
    uint8_t data7 = (IPMETA1 & 0xff000000000000) >>  48;
    uint8_t data8 = (IPMETA1 & 0xff00000000000000) >>  56;

    int32_t ret = 0, i = 0, d1 = 0;

    for (i = 0 ; i < 4; i ++) {
      	d1 = (*(a + i) >>  0 & 0xff);

  	if (data1 == d1) ret |= 0x02 << (i *8);
  	else if (data2 == d1) ret |= 0x08 << (i *8);
  	else if (data3 == d1) ret |= 0x20 << (i *8);
  	else if (data4 == d1) ret |= 0x80 << (i *8);
  	else if (data5 == d1) ret |= 0x08 << (i *8);
  	else if (data6 == d1) ret |= 0x20 << (i *8);
  	else if (data7 == d1) ret |= 0x80 << (i *8);
  	else if (data8 == d1) ret |= 0x80 << (i *8);

	//printf("\n i %d data: %x-%x-%x-%x-%x-%x-%x-%x a+i %x", i, data1, data2, data3, data4, data5, data6, data7, data8,  *(a+i) & 0xff);
    }

    return ret;
}

__attribute__((always_inline)) static inline int cmpEthType(const uint64_t *a)
{
    uint16_t data1 = (ETHMETA1 & 0xffff) >>  0;
    uint16_t data2 = (ETHMETA1 & 0xffff0000) >> 16;
    uint16_t data3 = (ETHMETA1 & 0xffff00000000) >>  32;
    uint16_t data4 = (ETHMETA1 & 0xffff000000000000) >>  48;

    int32_t ret = 0, i = 0, d1 = 0, d2 = 0, d3 = 0, d4 =0;

    for (i = 0 ; i < 4; i ++) {
      	d1 = (*(a + i) >>  0 & 0xffff);
      	d2 = (*(a + i) >> 16 & 0xffff);
      	d3 = (*(a + i) >> 32 & 0xffff);
      	d4 = (*(a + i) >> 48 & 0xffff);

  	if (data1 == d1) ret |= 0x02 << (i *8);
  	else if (data2 == d1) ret |= 0x08 << (i *8);
  	else if (data3 == d1) ret |= 0x20 << (i *8);
  	else if (data4 == d1) ret |= 0x80 << (i *8);

	//printf("\n i %d data 1: %x 2: %x 3: %x 4: %x a+i %x", i, data1, data2, data3, data4, *(a+i)  & 0xffff);
    }

    return ret;
}

 __attribute__((always_inline)) static inline int cmpIpType_avx2(const uint64_t *a)
{
        return _mm256_movemask_epi8 (_mm256_cmpeq_epi8((__m256i){*(a+0), *(a+1), *(a+2), *(a+3)}, (__m256i){IPMETA1, IPMETA1, IPMETA1, IPMETA1}));
}

 __attribute__((always_inline)) static inline int cmpIpType_sse(const uint64_t *a)
{
	return _mm_movemask_epi8(_mm_cmpeq_epi8 ((__m128i) {*(a+0), *(a+1)}, (__m128i) {IPMETA1, IPMETA1}));
}

 __attribute__((always_inline)) static inline int cmpEthType_sse(const uint64_t *a)
{
	return _mm_movemask_epi8( _mm_and_si128(_mm_cmpeq_epi16 ((__m128i) {*(a+0), *(a+1)}, (__m128i) {ETHMETA1, ETHMETA1}), (__m128i){0xf000f000f000f000, 0xf000f000f000f000}));
}

 __attribute__((always_inline)) static inline int cmpEthType_avx2(const uint64_t *a)
{
        return _mm256_movemask_epi8 (_mm256_and_si256(_mm256_cmpeq_epi16((__m256i){*(a+0), *(a+1), *(a+2), *(a+3)}, (__m256i){ETHMETA1, ETHMETA1, ETHMETA1, ETHMETA1}), (__m256i){0xf000f000f000f000, 0xf000f000f000f000, 0xf000f000f000f000, 0xf000f000f000f000}));
}

 __attribute__((always_inline)) static inline void counter_eth(uint64_t *counters, uint64_t *t2)
{
  __m256i count1 =  _mm256_maskload_epi64 ((long long int const *)counters +  0, _mm256_set1_epi64x(0x8000000000000000));
  uint16_t *m = (uint16_t *) t2;
  count1 = _mm256_add_epi64(count1, _mm256_set_epi64x(m[0] &0x1, m[1] & 0x1, m[2] & 0x1, m[3] & 0x1));
  _mm256_store_si256((__m256i *)counters + 0, count1);
  return;
}

 __attribute__((always_inline)) static inline void counter_ip(uint64_t *counters, uint64_t *t2)
{
  __m256i count1 =  _mm256_maskload_epi64 ((long long int const *)counters, _mm256_set1_epi64x(0x8000000000000000));
  uint8_t *m = (uint8_t *)t2;
  count1 = _mm256_add_epi64(count1, _mm256_set_epi64x(m[0] &0x1, m[1] & 0x1, m[2] & 0x1, m[3] & 0x1));
  _mm256_store_si256((__m256i *)counters, count1);
  return;
}


 __attribute__((always_inline)) static inline void testmain_eth(const short *a, const uint64_t *counters)
{
#if 0
  return(_mm_movemask_epi8(_mm_cmpeq_epi16(_mm_set_epi64x (ETHMETA1, ETHMETA1), _mm_set1_epi16(a))));
#endif

  //__m256i count =  _mm256_load_si256 ((__m256i const *)counters);
  __m256i count1 =  _mm256_maskload_epi64 ((long long int const *)counters +  0, CNTMASK);
  __m256i count2 =  _mm256_maskload_epi64 ((long long int const *)counters +  4, CNTMASK);
  __m256i count3 =  _mm256_maskload_epi64 ((long long int const *)counters +  8, CNTMASK);
  __m256i count4 =  _mm256_maskload_epi64 ((long long int const *)counters + 12, CNTMASK);
#if 0
  __m256i t1 = _mm256_set_epi64x(ETHMETA1, ETHMETA1, ETHMETA1, ETHMETA1);
  __m256i t2 = _mm256_cmpeq_epi16(_mm256_set1_epi16(a), t1);
#else
  __m256i t2= (_mm256_cmpeq_epi16((__m256i) {*a,*a,*a,*a}, (__m256i) {ETHMETA1, ETHMETA1, ETHMETA1, ETHMETA1}));
#endif
  uint16_t *m = (uint16_t *) &t2;
  count1 = _mm256_add_epi64(count1, _mm256_set_epi64x(m[0] &0x1, m[1] & 0x1, m[2] & 0x1, m[3] & 0x1));
  count2 = _mm256_add_epi64(count2, _mm256_set_epi64x(m[4] &0x1, m[5] & 0x1, m[6] & 0x1, m[7] & 0x1));
  count3 = _mm256_add_epi64(count3, _mm256_set_epi64x(m[8] &0x1, m[9] & 0x1, m[10] & 0x1, m[11] & 0x1));
  count4 = _mm256_add_epi64(count4, _mm256_set_epi64x(m[12] &0x1, m[13] & 0x1, m[14] & 0x1, m[15] & 0x1));
  _mm256_store_si256((__m256i *)counters + 0, count1);
  _mm256_store_si256((__m256i *)counters + 4, count2);
  _mm256_store_si256((__m256i *)counters + 8, count3);
  _mm256_store_si256((__m256i *)counters + 12, count4);

  return;
}

 __attribute__((always_inline)) static inline void testmain_ip(const char *a, uint64_t *counters)
{
#if 0
  //return(_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set_epi64x (IPMETA1, IPMETA1), _mm_set1_epi8(a))));
#endif
  //__m256i count1 =  _mm256_load_si256 ((__m256i const *)counters);
  //__m256i count2 =  _mm256_load_si256 ((__m256i const *)counters + 1);
#if 0
  __m256i count1 =  _mm256_maskload_epi64 ((long long int const *)counters +  0, _mm256_set1_epi64x(0x8000000000000000));
  __m256i count2 =  _mm256_maskload_epi64 ((long long int const *)counters +  4, _mm256_set1_epi64x(0x8000000000000000));
  __m256i count3 =  _mm256_maskload_epi64 ((long long int const *)counters +  8, _mm256_set1_epi64x(0x8000000000000000));
  __m256i count4 =  _mm256_maskload_epi64 ((long long int const *)counters + 12, _mm256_set1_epi64x(0x8000000000000000));
  __m256i count5 =  _mm256_maskload_epi64 ((long long int const *)counters + 16, _mm256_set1_epi64x(0x8000000000000000));
  __m256i count6 =  _mm256_maskload_epi64 ((long long int const *)counters + 20, _mm256_set1_epi64x(0x8000000000000000));
  __m256i count7 =  _mm256_maskload_epi64 ((long long int const *)counters + 24, _mm256_set1_epi64x(0x8000000000000000));
  __m256i count8 =  _mm256_maskload_epi64 ((long long int const *)counters + 28, _mm256_set1_epi64x(0x8000000000000000));
#else
  __m256i count1 =  _mm256_maskload_epi64 ((long long int const *)counters +  0, CNTMASK);
  __m256i count2 =  _mm256_maskload_epi64 ((long long int const *)counters +  4, CNTMASK);
  __m256i count3 =  _mm256_maskload_epi64 ((long long int const *)counters +  8, CNTMASK);
  __m256i count4 =  _mm256_maskload_epi64 ((long long int const *)counters + 12, CNTMASK);
  __m256i count5 =  _mm256_maskload_epi64 ((long long int const *)counters + 16, CNTMASK);
  __m256i count6 =  _mm256_maskload_epi64 ((long long int const *)counters + 20, CNTMASK);
  __m256i count7 =  _mm256_maskload_epi64 ((long long int const *)counters + 24, CNTMASK);
  __m256i count8 =  _mm256_maskload_epi64 ((long long int const *)counters + 28, CNTMASK);
#endif
#if 0
  __m256i t1 = _mm256_set_epi64x(IPMETA1, IPMETA1, IPMETA1, IPMETA1);
  __m256i t2 = _mm256_cmpeq_epi8(_mm256_set1_epi8(a), t1);
#else
  __m256i t2 = _mm256_cmpeq_epi8((__m256i) {*a,*a,*a,*a}, (__m256i) {IPMETA1, IPMETA1, IPMETA1, IPMETA1});
#endif
  uint8_t *m = (uint8_t *)&t2;
  count1 = _mm256_add_epi64(count1, _mm256_set_epi64x((m[0] &0x1), (m[1] & 0x1), (m[2] & 0x1), (m[3] & 0x1)));
  count2 = _mm256_add_epi64(count2, _mm256_set_epi64x((m[4] &0x1), (m[5] & 0x1), (m[6] & 0x1), (m[7] & 0x1)));
  count3 = _mm256_add_epi64(count2, _mm256_set_epi64x((m[8] &0x1), (m[9] & 0x1), (m[10] & 0x1), (m[11] & 0x1)));
  count4 = _mm256_add_epi64(count2, _mm256_set_epi64x((m[12] &0x1), (m[13] & 0x1), (m[14] & 0x1), (m[15] & 0x1)));
  count5 = _mm256_add_epi64(count2, _mm256_set_epi64x((m[16] &0x1), (m[17] & 0x1), (m[18] & 0x1), (m[19] & 0x1)));
  count6 = _mm256_add_epi64(count2, _mm256_set_epi64x((m[20] &0x1), (m[21] & 0x1), (m[22] & 0x1), (m[23] & 0x1)));
  count7 = _mm256_add_epi64(count2, _mm256_set_epi64x((m[24] &0x1), (m[25] & 0x1), (m[26] & 0x1), (m[27] & 0x1)));
  count8 = _mm256_add_epi64(count2, _mm256_set_epi64x((m[28] &0x1), (m[29] & 0x1), (m[30] & 0x1), (m[31] & 0x1)));

  _mm256_store_si256((__m256i *)counters, count1);
  _mm256_store_si256((__m256i *)counters + 1, count2);

  return;
}


#if 0
int main()
{
        int32_t ret = 0;
    	uint64_t ethType[4] = {0};
        uint64_t ipType[2] = {0};

#if 1
       	ethType[0] = 0x0800080008000800; 
	ethType[1] = 0x86dd86dd86dd86dd;
	ethType[2] = 0x8100810081008100;  	
	ethType[3] = 0x0806080608060806;     
	//ret = cmpEthType_avx2(&ethType[0]);
	//printf(" mask ret: %x\n", ret);
        ret = cmpEthType(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_sse(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_avx2(&ethType[0]);    
        printf(" mask ret: %x\n", ret);

    	ethType[0] = 0x0800080008000800;
    	ethType[1] = 0x0800080008000800;
    	ethType[2] = 0x0800080008000800;
    	ethType[3] = 0x0800080008000800;
    	//ret = cmpEthType_avx2(&ethType[0]);
        //printf(" mask ret: %x\n", ret);
        ret = cmpEthType(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_sse(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_avx2(&ethType[0]);    
        printf(" mask ret: %x\n", ret);

    	ethType[0] = 0x86dd86dd86dd86dd;
    	ethType[1] = 0x86dd86dd86dd86dd;
    	ethType[2] = 0x86dd86dd86dd86dd;
    	ethType[3] = 0x86dd86dd86dd86dd;
    	//ret = cmpEthType_avx2(&ethType[0]);
        //printf(" mask ret: %x\n", ret);
        ret = cmpEthType(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_sse(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_avx2(&ethType[0]);    
        printf(" mask ret: %x\n", ret);

    	ethType[0] = 0x8100810081008100;
    	ethType[1] = 0x8100810081008100;
    	ethType[2] = 0x8100810081008100;
    	ethType[3] = 0x8100810081008100;
    	//ret = cmpEthType_avx2(&ethType[0]);
        //printf(" mask ret: %x\n", ret);
        ret = cmpEthType(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_sse(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_avx2(&ethType[0]);    
        printf(" mask ret: %x\n", ret);

    	ethType[0] = 0x0806080608060806;
    	ethType[1] = 0x0806080608060806;
    	ethType[2] = 0x0806080608060806;
    	ethType[3] = 0x0806080608060806;
    	//ret = cmpEthType_avx2(&ethType[0]);
        //printf(" mask ret: %x\n", ret);
        ret = cmpEthType(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_sse(&ethType[0]);    
        printf(" mask ret: %x\n", ret);
        ret = cmpEthType_avx2(&ethType[0]);    
        printf(" mask ret: %x\n", ret);

    	//ret = (CMP_ETHTYPE_AVX2(&ethType[0]));
        //ret = cmpEthType(&ethType[0]);    
#else
       	ipType[0] = 0x1111111111111111; 
	ipType[1] = 0x0606060606060606;
    	ret = cmpIpType_sse(&ipType[0]);
        printf(" mask ret: %x\n", ret);
#endif

      	return 0;
  
}
#endif

