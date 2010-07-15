#include <sys/time.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define insidetasks

typedef int t_int;

typedef int word_t;
//typedef __uint128_t word_t;
//typedef __uint64_t word_t;
//typedef __uint32_t word_t;

//CG rsort
#define fr(x,y)for(int x=0;x<y;x++)



double maintime_int(int print) {

        double elapsed_seconds = 0;
        static struct timeval t1; /* var for previous time stamp */
        static struct timeval t2; /* var of current time stamp */

        if (gettimeofday(&t2, NULL) == -1) {
                perror("gettimeofday");
                exit(9);
        }

        if (print) {
                elapsed_seconds = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) * 1e-6;
                printf("Time spent [%.2fs] \n", elapsed_seconds);
        }

        t1 = t2;
        return elapsed_seconds;
}






//task1 - independent - input NN inout inputArray
#pragma css task input (size, offset) \
                 output (array[size])

void init_inputArray_task(t_int *array, int size, int offset) {
	int i;
	for(i=0;i < size; i++)
		array[i] = i+offset;
}

//task2 - independent - input RANGE inout counters
#pragma css task input (size, offset) \
	             output (counters[size])

void init_counters_task (t_int *counters, int size, int offset) {
	int k;
	for(k=0; k < size; k++)
		counters[k] = 0;
}

//task3 histogram - reduction - input NN, buffer inout counters
#pragma css task input (size, offset, cntsize, bufsize, buffer[bufsize]) inout (counters[cntsize]) reduction (counters[cntsize])

void counters_load_task(t_int *counters, int cntsize, unsigned char *buffer, int bufsize, int size, int offset) {
	int i;

	t_int localcounters[cntsize];
	memset(localcounters, 0, cntsize * sizeof(t_int));

	for(i=offset; i < size; i++) {
		localcounters[*(buffer + i)]++;
	}
#pragma css mutex lock (counters)
	for(i=0; i < cntsize; i++)
		counters[i] += localcounters[i];
#pragma css mutex unlock (counters)

}

//task4 - nonparallel
#pragma css task input (size, counters[size]) \
	             output (startpos[size])

void init_startpos_task(t_int *startpos, t_int *counters, int size) {
	t_int sp = 0;
	int k;

	for(k = 0; k < size; k++) {
		startpos[k] = sp;
		sp += counters[k];
	}
}
/*
//task 5 major loop
#pragma css task input (size, ofs, bufsize, OFFSET, CPYCHAR, NUMCPYCHAR) \
				 input (buffer[bufsize], inputArray[size]) \
	             output (outputArray[NUMCPYCHAR])

void load_outputArray_task(t_int *inputArray, t_int *outputArray, unsigned char *buffer, int bufsize, int size, int ofs, int OFFSET, unsigned char CPYCHAR, int NUMCPYCHAR) {
	int i;
	int offset_counter = 0;
	int copy_counter = 0;

	for(i = 0; i < size; i++) {
		unsigned char c = buffer[ofs + inputArray[i]];
		if (c == CPYCHAR) {
			if(offset_counter < OFFSET)
				offset_counter++;
			else {
				outputArray[copy_counter] = inputArray[i];
				copy_counter++;
				if(copy_counter == NUMCPYCHAR)
					break;
			}
		}
	}
}
*/

//task 5 major loop
#pragma css task input (size, ofs, bufsize, OFFSET, CPYCHAR, NUMCPYCHAR) \
				 input (buffer[bufsize], inputArray[size]) \
	             output (outputArray[NUMCPYCHAR])

void load_outputArray_task(t_int *inputArray, t_int *outputArray, unsigned char *buffer, int bufsize, int size, int ofs, int OFFSET, unsigned char CPYCHAR, int NUMCPYCHAR) {
	int i;
	int offset_counter = 0;
	int copy_counter = 0;

	for(i = 0; i < size; i++) {
		unsigned char c = buffer[ofs + inputArray[i]];
		if (c == CPYCHAR) {
			if(offset_counter < OFFSET)
				offset_counter++;
			else {
				outputArray[copy_counter] = inputArray[i];
				copy_counter++;
				if(copy_counter == NUMCPYCHAR)
					break;
			}
		}
	}
}



#pragma css task input (SIZE, typeSize, orig[SIZE]) \
                 output (dst[SIZE])

void memcpy_task(t_int *dst, t_int *orig, int SIZE, int typeSize){
	memcpy(dst, orig, SIZE * typeSize);
}



#ifndef insidetasks
#pragma css task input (buffer[numlines], numlines, DEPTH) \
	             output (index[numlines])
#endif

void simpler_rsort4ngrams(unsigned char *buffer, int numlines, int DEPTH, int *index) {
	int NN = numlines - DEPTH + 1;

	if (NN > 0) {
		unsigned char *pin = buffer + NN;
		unsigned char *pout = buffer;
		typedef int t_int;
		t_int *inputArray = (t_int*) malloc(NN * sizeof(t_int));
		t_int *outputArray = (t_int*) malloc(NN * sizeof(t_int));
		const int RANGE = 256;
		t_int counters[RANGE];
		t_int startpos[RANGE];

		int i, j, k;

#ifndef insidetasks

		for(i=0;i < NN; i++)
			inputArray[i] = i;
#else
		//task1 - independent - input NN inout inputArray
		int block_records = 500000;
		int chunk_of_records = 0;

		for (k = 0; k < NN; k += block_records) {
			chunk_of_records = (k + block_records > NN ? NN - k : block_records);
			init_inputArray_task(&inputArray[k], chunk_of_records, k);
		}
#endif
		//radix sort, the input is x, the output rank is ix
		//counters

#ifndef insidetasks

		for(k=0; k < RANGE; k++)
			counters[k] = 0;

#else

		//task2 - independent - input RANGE inout counters
		init_counters_task(counters, RANGE, 0);
#endif

#ifndef insidetasks

		for(i=0; i < NN; i++)
			counters[*(buffer + i)]++;

#else

		//task3 histogram - reduction - input NN, buffer inout counters
		for (k = 0; k < NN; k += block_records) {
			chunk_of_records = (k + block_records > NN ? NN - k : block_records);
			counters_load_task(counters, RANGE, buffer, numlines, k+chunk_of_records, k);
		}

#endif

		for(j=0; j < DEPTH; j++) {
			int ofs = j;//low endian
			t_int sp = 0;

#ifndef insidetasks

			for(k = 0; k < RANGE; k++) {
				startpos[k] = sp;
				sp += counters[k];
			}

#else

			//task 4
			init_startpos_task(startpos, counters, RANGE);
#endif

#ifndef insidetasks

			for(i = 0; i < NN; i++) {
				unsigned char c = buffer[ofs + inputArray[i]];
				outputArray[startpos[c]++] = inputArray[i];
			}
#else
			//task 5 major loop - task reduction

/*#pragma css wait on (outputArray, inputArray)

			for (k = 0; k < NN; k += block_records) {
				chunk_of_records = (k + block_records > NN ? NN - k : block_records);
				load_outputArray_task(&inputArray[k], outputArray, startpos, RANGE, buffer, numlines, chunk_of_records, ofs);
			}
*/
#pragma css barrier

			for (i = 0; i < RANGE; i++) {
				for(k = 0; k < counters[i]; k+= block_records) {
					chunk_of_records = (k + block_records > counters[i] ? counters[i] - k : block_records);
					load_outputArray_task(inputArray, &outputArray[startpos[i]+k], buffer, numlines, NN, ofs, k, i, chunk_of_records);
				}
			}
#pragma css barrier

#endif
#ifndef insidetasks

			memcpy(inputArray, outputArray, NN * sizeof(inputArray[0]));

#else

#pragma css wait on (outputArray)

			for (k = 0; k < NN; k += block_records) {
				chunk_of_records = (k + block_records > NN ? NN - k : block_records);
				memcpy_task(&inputArray[k], &outputArray[k], chunk_of_records, sizeof(inputArray[0]));
			}
#pragma css barrier
#endif

			//update counters
			if (j < DEPTH - 1) {
				counters[*pout++]--;
				counters[*pin++]++;
			}

		}

#ifndef insidetasks

		memcpy(index, inputArray, NN * sizeof(inputArray[0]));
#else

#pragma css wait on (inputArray)

		for (k = 0; k < NN; k += block_records) {
			chunk_of_records = (k + block_records > NN ? NN - k : block_records);
			memcpy_task(&index[k], &inputArray[k], chunk_of_records, sizeof(inputArray[0]));
		}
#endif
		free(outputArray);
	} else
		index = NULL;
}

#define MAXBUFSIZ 8000123
unsigned char bufferfile1[MAXBUFSIZ];
unsigned char bufferfile2[MAXBUFSIZ];
int numlines1, numlines2;

inline word_t readat(const unsigned char *buf, int poz) {
	return *(word_t *) (buf + poz);
}

int main(int argc, char ** argv) {
	int depth = sizeof(word_t);
	int cnt = 0;

	FILE *fd1 = fopen(argv[1], "rb");
	numlines1 = fread(bufferfile1, 1, MAXBUFSIZ, fd1);
	fclose(fd1);
	FILE *fd2 = fopen(argv[2], "rb");
	numlines2 = fread(bufferfile2, 1, MAXBUFSIZ, fd2);
	fclose(fd2);

	maintime_int(0);

#pragma css start

	//index the ngrams
	t_int *index1 = (t_int*) malloc(numlines1 * sizeof(t_int));
	t_int *index2 = (t_int*) malloc(numlines2 * sizeof(t_int));

	simpler_rsort4ngrams(bufferfile1, numlines1, depth, index1);
	simpler_rsort4ngrams(bufferfile2, numlines2, depth, index2);

#pragma css barrier

	maintime_int(1);

	//merge
	int linecnt1 = 0;
	int linecnt2 = 0;
	numlines1 -= (depth - 1);
	numlines2 -= (depth - 1);
	if (index1 && index2) {
		word_t str1 = readat(bufferfile1, index1[linecnt1]);
		word_t str2 = readat(bufferfile2, index2[linecnt2]);

		while (linecnt1 < numlines1 && linecnt2 < numlines2) {
			//		fprintf(stderr,"i1=%d i2=%d s1=%0llx s2=%0llx\n",i1,i2,s1,s2);
			if (str1 == str2) {
//				printf("%d %d\n", index1[linecnt1], index2[linecnt2]);
				cnt++;
				linecnt1++;
				if (linecnt1 < numlines1)
					str1 = readat(bufferfile1, index1[linecnt1]);
				linecnt2++;
				if (linecnt2 < numlines2)
					str2 = readat(bufferfile2, index2[linecnt2]);
			} else if (str1 < str2) {
				linecnt1++;
				if (linecnt1 < numlines1)
					str1 = readat(bufferfile1, index1[linecnt1]);
			} else if (str2 < str1) {
				linecnt2++;
				if (linecnt2 < numlines2)
					str2 = readat(bufferfile2, index2[linecnt2]);
			}
		}
		free(index2);
		free(index1);
	}
#pragma css finish
	maintime_int(1);
printf(">>>> cnt = %d\n\n", cnt);
	return 0;
}
