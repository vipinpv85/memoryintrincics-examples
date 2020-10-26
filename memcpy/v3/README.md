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
 2. avx2 CPU
 3. pthread
 4. AGNER vector class library
 
 note: please update the path in Makefile to include.
 
 # RESULT
