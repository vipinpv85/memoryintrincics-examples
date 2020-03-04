#include <smmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

__attribute__((noinline)) int mysse_v10(const adk_ipv6_dp_uli_flow_cache_entry_t * restrict lut, const unsigned char * restrict pkt, const uint16_t count)
{
        register int i = 0;

        register __m128i pkt_dstip;
        register __m128i cmp_result_1;
        register uint64_t pkt_dstport_sockid;

        register uint8_t *lut_ptr;
        
        if (count == 0)
          return -1;

        pkt_dstip = _mm_loadu_si128((__m128i const*) ((unsigned char *)pkt + 8));
        pkt_dstport_sockid = (*((uint16_t *)((unsigned char *)pkt + 42)) << 0 | *((uint32_t *)(unsigned char *)pkt + 128) << 16) & 0xffffffffff;

        for (; i < count; i++) {
                //cmp_result_1 = _mm_xor_si128(pkt_dstip, _mm_loadu_si128((__m128i const*)((uint8_t *)lut[i].dst_addr)));
                cmp_result_1 = _mm_xor_si128(pkt_dstip, _mm_load_si128((__m128i const*)((uint8_t *)lut[i].dst_addr)));

                if (!!(_mm_testz_si128 (cmp_result_1, cmp_result_1) == 1)) {

                        if (pkt_dstport_sockid == (*(uint64_t *)&lut[i].dst_port & 0xffffffffff))
                                return i;
                }
        }

        return -1;
}
