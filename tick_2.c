        unsigned int tickl, tickh;
        __m128i *ptr = NULL, res1, res2, res3;
        uint64_t b[8];
        ptr = (__m128i *) b;

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

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        start = ((unsigned long long)tickh << 32)|tickl;

        for (uint32_t i = 10; i > 0; i--)
        {
                memset((uint64_t *)b, 0, 8 * 64);
        }

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        end = ((unsigned long long)tickh << 32)|tickl;

        printf("end - start memset %llu\n", end - start);

		
        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        start = ((unsigned long long)tickh << 32)|tickl;

        *((__m128i *) eth) = *((__m128i *) l2fwd_ports_eth_addr[dest_portid].addr_bytes);

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        end = ((unsigned long long)tickh << 32)|tickl;
        printf("time taken for simd %llu\n", end - start);

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        start = ((unsigned long long)tickh << 32)|tickl;

        /* src addr */
        ether_addr_copy(&l2fwd_ports_eth_addr[dest_portid], &eth->s_addr);

        __asm__ __volatile__("rdtsc" : "=a"(tickl), "=d"(tickh));
        end = ((unsigned long long)tickh << 32)|tickl;
        printf("time taken for ether_addr_copy %llu\n", end - start);
