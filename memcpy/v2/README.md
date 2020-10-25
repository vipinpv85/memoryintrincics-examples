# Build

```
`make` | default 
`make BUILD=release` | highest performance.
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
 Thread-000: v1 (7421657572), v2 ( 995775429), v8( 390357542)
 Thread-001: v1 (8384317872), v2 ( 886572582), v8( 409136270)
 Thread-002: v1 (8142762369), v2 ( 840387737), v8( 381935822)
 Thread-003: v1 (8231782182), v2 ( 799853321), v8( 379360412)
 Thread-004: v1 (6856121008), v2 ( 944510834), v8( 534521181)
 Thread-005: v1 (6897667357), v2 ( 963315366), v8( 525338890)
 Thread-006: v1 (6869621941), v2 ( 951395149), v8( 494752371)
 Thread-007: v1 (6972086442), v2 ( 931046481), v8( 524303991)
 --- with Pstate ticks can be unreliable ---
 Total: 59776016743, 7312856899, 3639706479 with (malloc)
 speedup %: 100.000000, 717.409912, 1542.330811
 ---------------------------------


 ```
