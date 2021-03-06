# memoryintrincics-examples

Sample use cases and exploration on use of SIMD on x86 (sse, ssse, avx). Most of use cases covered are reducing compute and instruction retired for checksum and lookup in networking.

# build:

## compilers: `gcc and icc`

## on x86
| category | command |
| -- | -- |
| `AVX` | gcc -mavx2 test.c |
| `SSE` | gcc -msse4.2 lut_test.c |
| `NASM` | nasm -f elf64 test.asm; cc test1.c test.o && ./a.out |


## detailed review
| tool | command |
| -- | -- |
| `perf` | perf stat -e task-clock,LLC-loads,LLC-load-misses,branches,branch-misses,faults,migrations,alignment-faults,uops_issued.stall_cycles,uops_executed.stall_cycles,uops_retired.stall_cycles,bus-cycles,L1-dcache-loads,L1-dcache-load-misses,L1-dcache-stores,lsd.uops,uops_issued.any,dTLB-loads,dTLB-load-misses,dTLB-prefetch-misses,bus-cycles,cpu-cycles,ref-cycles,cycles:u,cycles:k,instructions,inst_retired.any,context-switches,cpu-migrations |
| `objdump` | objdump -S --start-address=`address of binary` `binary file` |
