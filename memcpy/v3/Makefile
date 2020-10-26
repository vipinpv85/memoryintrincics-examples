# to build with debug, execute make BUILD=debug
# to enable fill random for 16GB, execute make BUILD=fill
# fill 16Gb with random value, execute make BUILD=fill
BUILD := release 

CC = gcc
cflags.common := -std=gnu11
CC = g++

cflags.debug := -I. -ggdb -O0 -m64  -fopt-info-vec-missed -ftree-vectorizer-verbose=2
cflags.release =-I. -Ofast -m64 -march=native 
cflags.fill =-I. -DFILLRNDM -Ofast -m64 -march=native 
CFLAGS := -Wall ${cflags.${BUILD}} ${cflags.common}

CFLAGS += -fno-plt -free
CFLAGS += -lnuma -pthread -mavx2 -fabi-version=0
#-mavx -msse -msse4.1 -msse4.2
#CFLAGS += -ftree-loop-distribution
CFLAGS += -ftree-vectorize
#CFLAGS += -fprefetch-loop-arrays
CFLAGS += -funroll-loops
#CFLAGS += -mfma -ffast-math
CFLAGS += -mprefetchwt1
#CFLAGS += -fPIC

SRC = runlogic.c main.c
CFLAGS += -std=gnu++1z -mavx2 -mfma -fabi-version=0 -I /home/saesrv02/Downloads/explore/version2-master/

.PHONY: all

all: 
	@echo $(CFLAGS)
	mkdir -p build
	$(CC) $(SRC) -o build/test $(CFLAGS)

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
	./build/test

#%.c:
#	echo "file - " $@
#	$(CC) $@.c $a.o $(CFLAGS)

clean:
	@echo " --- invoking clean --- "
	swapoff -a
	rm -f  /swapfile
	rm -rf build
	sync;echo 3 > /proc/sys/vm/drop_caches ; free -m
