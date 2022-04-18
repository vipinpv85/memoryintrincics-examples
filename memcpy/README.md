# Enhanced MEMCPY-MEMCMP

## Motivation
For large data movement between memory area, requires data to copied and compared.
Using conventional `memcpy` and `memcmp` with libc might not be best option on x86 server processor.

## Solution
Rewriting with SIMD and multiple threads helps to reduce overall memcpy and memcmp.

# how to build and run
 - make clean
 - make all
 - make run or
 - ./build/test
