# memoryintrincics-examples
exploration with sample on the use of simd (sse, ssse, avx) for accelerating comparisons and compute

to build: gcc  -mavx2 test.c
perf: perf stat perf stat -Bd ./a.out

Note: to make use of simd, once can use asm, intrinics or compiler optimzation. We are building with no optimization and checked with objdump -d a.out
