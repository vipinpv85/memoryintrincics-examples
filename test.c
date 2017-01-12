#include <stdio.h>
#include "simd-helpers.h"

//#define count
//#define cmp
#define cmpcount

uint64_t eth[16]= {0, 0, 0, 0};
uint64_t  ip[32]= {0, 0, 0, 0, 0, 0, 0, 0};

uint64_t counteth = 0x80;
uint64_t countip = 0x8;


int main (void) 
{
 unsigned int i = 0;
 int ret = 0;
 uint16_t a = 0x800, b =0x86dd, c = 0x8100;
 uint64_t m[4] = {0x1111111111111111, 0x1111111111111111, 0x1111111111111111, 0x1111111111111111};


for (i = 1; i < 0xffffff; i++) {
 	ret = cmpIpType   (&m[0]);
 	//printf("\n norm: %x", ret);
 	ret = naiveChecksum(&m[0], 8);
 	//printf("\n naiveChecksum: %x", ret);

 	//ret = cmpIpType_sse  (&m[0]);
 	//ret = cmpIpType_avx2 (&m[0]);
 	//printf("\n avx: %x", ret);
 	//ret = avxChecksumV1(&m[0], 8);
 	//printf("\n avxChecksumV1: %x", ret);
}

#if 0
 printf("\n eth 16 stats: %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64, eth[0], eth[1], eth[2], eth[3], eth[4], eth[5], eth[6], eth[7], eth[8], eth[9], eth[10], eth[11], eth[12], eth[13], eth[14], eth[15]);
 printf("\n ip 0-7 stats: %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64, ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7]);
 printf("\n ip 8-15 stats: %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64, ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
 printf("\n ip 16-24 stats: %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64, ip[15], ip[16], ip[17], ip[18], ip[19], ip[20], ip[21], ip[22]);
 printf("\n ip 25-32 stats: %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64" %"PRIx64, ip[24], ip[25], ip[26], ip[27], ip[28], ip[29], ip[30], ip[31]);
#endif

 return 0;
}

