# to build with debug, execute make BUILD=debug                                                                                                                                                                                     [10/1853]
# to enable fill random for 16GB, execute make BUILD=fill
# fill 16Gb with random value, execute make BUILD=fill
BUILD := release

CC = gcc
CPP = g++

cflags.common := -std=gnu11
cflags.debug := -I. -ggdb -O0 -m64
cflags.release =-I. -ggdb -Ofast -m64 -march=native
cflags.fill =-I. -DFILLRNDM -ggdb -Ofast -m64 -march=native

CFLAGS := ${cflags.common}
CFLAGS += ${cflags.${BUILD}}
#CFLAGS += -fopt-info-vec-missed -ftree-vectorizer-verbose=2
#CFLAGS += -fno-plt -free -gdwarf
CFLAGS += -lnuma -pthread -mavx2
CFLAGS += -funroll-loops -ftree-loop-distribution
CFLAGS += -mprefetchwt1 -ftree-vectorize -mavx2  -fprefetch-loop-arrays -funroll-loops -mfma -ffast-math

SRC = main.c
OBJ = main.o


.PHONY: all

all: $(OBJ)
        mkdir -p build
        $(CC) $(OBJ) -o build/test $(CFLAGS)
        rm -rf $(OBJ)

$(OBJ): %.o: %.c
        @echo " input: $<"
        @echo " output: $@"
        $(CC) -c $< -fPIC $(CFLAGS)

swap:
        swapoff -a
        fallocate -l 32G /swapfile
        chmod 600 /swapfile
        mkswap /swapfile
        swapon /swapfile
        ls -lh /swapfile
        swapon --show
        free -h

run:
        ulimit -v unlimited
        ulimit -a
        ulimit -v 0
        ulimit -v -m
        echo 2 > /proc/sys/vm/overcommit_memory
        @echo "------------------------------"
        @echo "commit ratio (total Memory * commit ratio = commit limit)"
        cat /proc/sys/vm/overcommit_ratio
        @echo "------------------------------"
        grep -e MemTotal -e CommitLimit -e Committed_AS /proc/meminfo
        ulimit -a
        @echo "------------------------------"
        sync;echo 3 > /proc/sys/vm/drop_caches ; free -m
        taskset -c 0-15 ./build/test

#%.c:
#       echo "file - " $@
#       $(CC) $@.c $a.o $(CFLAGS)

clean:
        @echo " --- invoking clean --- "
        rm -rf build
        sync;echo 3 > /proc/sys/vm/drop_caches ; free -m
