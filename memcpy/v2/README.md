# Commands

```
`make` | default 
`make BUILD=release` | highest performance.
`make BUILD=debug` | build with debug flags to run with gdb
`make BUILD=fill` | build in release mode with logic to fill 16GB src with random values
`make swap` | builds swap partition with 32GB
`make clean` | cleans up the enviroment
`make run` | execute the ELF
```

## dependancy

1. libnuma
2. avx256
3. pthread

## build with gcc

1. Comment line 8 & 9

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
 Thread-000: v1 (7509729505), v2 ( 951816594), v8( 423271368)
 Thread-001: v1 (7830170666), v2 ( 943399347), v8( 371518478)
 Thread-002: v1 (8332572442), v2 ( 843473692), v8( 370875212)
 Thread-003: v1 (8382257163), v2 ( 833805654), v8( 372627416)
 Thread-004: v1 (6761390410), v2 ( 864660074), v8( 455198334)
 Thread-005: v1 (7025199841), v2 ( 956282406), v8( 466104471)
 Thread-006: v1 (7037096255), v2 ( 966897785), v8( 463191139)
 Thread-007: v1 (6857233828), v2 ( 884040548), v8( 450899658)
 --- with Pstate ticks can be unreliable ---
 Total: 59735650110, 7244376100, 3373686076 with (malloc)
 speed-up %: 100.000000, 724.579651, 1670.634521
 ---------------------------------
 ```
