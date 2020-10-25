# Build

```
`make` | default mode is release
`make BUILD=debug` | build with debug flags to run with gdb
`make BUILD=fill` | build in release mode with logic to fill 16GB src with random values
`make swap` | builds swap partition with 32GB
`make clean` | cleans up the enviroment
`make run` | execute the ELF
```
 
 # dependancy
 1. libnuma
 2. avx256
 3. pthread
 
 # RESULT
 
 ```
 we have 88 CPU, run test! RUN: CPU requested (8)
 RUN: MEMORY requested (32 GB)
 index-0: (./build/test)
 allocated 16GB src and 16GB dst with malloc!
src 0x7f7879266010 dst 0x7f7479265010
 Enter do_CopyCompare for 0!
 Enter do_CopyCompare for 1!
 Enter do_CopyCompare for 2!
 Enter do_CopyCompare for 3!
 Enter do_CopyCompare for 4!
 Enter do_CopyCompare for 5!
 Enter do_CopyCompare for 6!
 Enter do_CopyCompare for 7!
 ptr 0x7f74f9265010 dst_len 0x7f7579265010
 ptr 0x7f7679265010 dst_len 0x7f76f9265010
 ptr 0x7f75f9265010 dst_len 0x7f7679265010
 ptr 0x7f7779265010 dst_len 0x7f77f9265010
 ptr 0x7f7579265010 dst_len 0x7f75f9265010
 ptr 0x7f77f9265010 dst_len 0x7f7879265010
 ptr 0x7f76f9265010 dst_len 0x7f7779265010
 ptr 0x7f7479265010 dst_len 0x7f74f9265010
 Exit do_CopyCompare 4!
 Exit do_CopyCompare 6!
 Exit do_CopyCompare 7!
 Exit do_CopyCompare 5!
 Exit do_CopyCompare 1!
 Exit do_CopyCompare 0!
 Exit do_CopyCompare 2!
 Exit do_CopyCompare 3!

---- Summary in ticks -----
 Thread-000: v1 (7818445170), v8( 351543936)
 Thread-001: v1 (7378289388), v8( 436029854)
 Thread-002: v1 (8435493277), v8( 354350380)
 Thread-003: v1 (8528818915), v8( 351968388)
 Thread-004: v1 (7159699321), v8( 504443516)
 Thread-005: v1 (7255083950), v8( 524790680)
 Thread-006: v1 (7202902665), v8( 527408362)
 Thread-007: v1 (7238351060), v8( 528255528)
 --- with Pstate ticks can be unreliable ---
 Total: 61017083746, 3578790644 with (malloc)
 speedup %: 100.000000, 1604.963745
 ---------------------------------

 ```
