        unsigned int tickl, tickh;
        __m128i a[4], *ptr = NULL, res1, res2, res3;
        uint64_t b[8];
        a[0]  = _mm_set1_epi8('A');
        a[1]  = _mm_set1_epi8('B');
        a[2]  = _mm_set1_epi8('C');
        a[3]  = _mm_set1_epi8('D');
        ptr = a;

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        unsigned long long start = ((unsigned long long)tickh << 32)|tickl;

        for (uint32_t i = 10; i > 0; i--)
        {
                ptr[0] = ptr[1] = ptr[2] = ptr[3] = _mm_setzero_si128();

                res1 = _mm_cmpeq_epi64 (ptr[0], ptr[1]);
                res2 = _mm_cmpeq_epi64 (ptr[2], ptr[3]);
        }

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        unsigned long long end = ((unsigned long long)tickh << 32)|tickl;

        printf("end - start %llu\n", end - start);
        res3 = _mm_cmpeq_epi64 (res1, res2);
        ptr = &res3;

        a[0]  = _mm_set1_epi8('A');
        a[1]  = _mm_set1_epi8('B');
        a[2]  = _mm_set1_epi8('C');
        a[3]  = _mm_set1_epi8('D');

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        start = ((unsigned long long)tickh << 32)|tickl;

        for (uint32_t i = 10; i > 0; i--)
        {
                memset((uint64_t *)a, 0, 8 * 64);
        }

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        end = ((unsigned long long)tickh << 32)|tickl;

        printf("end - start memset %llu\n", end - start);
