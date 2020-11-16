#include "runlogic.h"

int main (int argc, char *argv[])
{
        int nthreads, tid;
        unsigned int i;
        uint8_t is_mmap = 0;
        char *src = NULL;
        char *dst = NULL;
        char *ptr = NULL;
        myThreadData_t myThreadData[128];

        assert(argc == 1);


        /* allocate src with 16GB */
        /* allocate dst with 16GB */
        src = (char *)malloc(TOTALSIZE);
        if (src != NULL) {
                dst = (char *)malloc(TOTALSIZE);
                if (dst != NULL)
                        fprintf(stdout, " allocated 16GB src and 16GB dst with malloc!\n");
                else {
                        fprintf(stderr, "FAIL to allocate dst from malloc using mmap\n");
                        free(src);
                        src = NULL;
                        dst = NULL;
                }
        } else {
                fprintf(stderr, "FAIL to allocate src from malloc using mmap\n");
                src = NULL;
                dst = NULL;
        }

        if ((src == NULL) && (dst == NULL)) {
                is_mmap = 1;

                src = (char *)mmap(NULL, TOTALSIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_NORESERVE, -1, 0);
                dst = (char *)mmap(NULL, TOTALSIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE|MAP_NORESERVE, -1, 0);

                if ((src == MAP_FAILED) || (dst == MAP_FAILED)) {
                        fprintf(stderr, "ERR: src %p dst %p MAP_FAILED\n", src, dst);
                        exit(2);
                }
        }

        assert(src);
        assert(dst);

        fprintf(stdout, " %s src %p dst %p \n", (is_mmap)?"MMAP":"MALLOC",  src, dst);

        #pragma omp parallel shared(nthreads)
        {
                nthreads = omp_get_num_threads();
                assert (NUMTHREADS <= nthreads);
        }

        for (i = 0; i < nthreads; i++) {
                myThreadData[i].len = (TOTALSIZE / nthreads);
                myThreadData[i].src = src + i * myThreadData[i].len;
                myThreadData[i].dst = dst + i * myThreadData[i].len;
                myThreadData[i].threadIndex = tid;
                myThreadData[i].tickCount = 0;
        }
        myThreadData[i - 1].len = (TOTALSIZE - (myThreadData[i - 1].len * (nthreads - 1)));

        #pragma omp parallel private(nthreads, tid)
        {
                tid = omp_get_thread_num();
                if (tid == 0) {
                        nthreads = omp_get_num_threads();
                        printf("Number of threads = %d\n", nthreads);
                        if (TOTALSIZE % nthreads) {
                                fprintf(stderr, "WARN: size %lu is not multiple of threads (%u)!\n", TOTALSIZE, nthreads);
                        }
                }

                printf("Thread %d starting...\n",tid);

                #pragma omp sections nowait
                {
                        #pragma omp section
                        {
                                printf("MEMCPY: TID %2d src %p, dst %p, len %lu \n", myThreadData[tid].threadIndex, myThreadData[tid].src, myThreadData[tid].dst, myThreadData[tid].len
);
                                ptr = memcpy((void *)myThreadData[tid].dst, (void *)myThreadData[tid].src, myThreadData[tid].len);
                                //assert(ptr == dst); /* to valdiate if copy has been completed */
                                printf(" ptr %p dst %p \n", ptr, dst);
                        }
                        nthreads = omp_get_num_threads();
                        printf("Number of threads = %d\n", nthreads);
                        if (TOTALSIZE % nthreads) {
                                fprintf(stderr, "WARN: size %lu is not multiple of threads (%u)!\n", TOTALSIZE, nthreads);
                        }
                }

                printf("Thread %d starting...\n",tid);

                #pragma omp sections nowait
                {
                        #pragma omp section
                        {
                                printf("MEMCPY: TID %2d src %p, dst %p, len %lu \n", myThreadData[tid].threadIndex, myThreadData[tid].src, myThreadData[tid].dst, myThreadData[tid].len
);
                                ptr = memcpy((void *)myThreadData[tid].dst, (void *)myThreadData[tid].src, myThreadData[tid].len);
                                //assert(ptr == dst); /* to valdiate if copy has been completed */
                                printf(" ptr %p dst %p \n", ptr, dst);
                        }

                        #pragma omp section
                        {
                                printf("MEMCMP: TID %2d src %p, dst %p, len %lu \n", myThreadData[tid].threadIndex, myThreadData[tid].src, myThreadData[tid].dst, myThreadData[tid].len
);
                                assert(memcmp(myThreadData[tid].dst, myThreadData[tid].src, myThreadData[tid].len) == 0);
                        }
                }

                printf("Thread %d done.\n",tid);
        }  /* end of parallel section */

        /* free src with 16GB */
        /* free dst with 16GB */
        fprintf(stdout, " freeing up %s src %p dst %p \n", (is_mmap)?"MMAP":"MALLOC", src, dst);
        if (is_mmap) {
                unlink("src");
                unlink("dst");
                munmap(src, TOTALSIZE);
                munmap(dst, TOTALSIZE);
        } else {
                free(src);
                free(dst);
        }
}

