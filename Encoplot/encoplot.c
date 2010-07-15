#include <sys/time.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef int t_int;

//typedef int word_t;
typedef __uint128_t word_t;
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




#pragma css task input (buffer[numlines], numlines, DEPTH) \
	             output (index[numlines])

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


		for(i=0;i < NN; i++)
			inputArray[i] = i;
		//radix sort, the input is x, the output rank is ix
		//counters


		for(k=0; k < RANGE; k++)
			counters[k] = 0;


		for(i=0; i < NN; i++)
			counters[*(buffer + i)]++;


		for(j=0; j < DEPTH; j++) {
			int ofs = j;//low endian
			t_int sp = 0;

			for(k = 0; k < RANGE; k++) {
				startpos[k] = sp;
				sp += counters[k];
			}

			for(i = 0; i < NN; i++) {
				unsigned char c = buffer[ofs + inputArray[i]];
				outputArray[startpos[c]++] = inputArray[i];
			}

			memcpy(inputArray, outputArray, NN * sizeof(inputArray[0]));

			//update counters
			if (j < DEPTH - 1) {
				counters[*pout++]--;
				counters[*pin++]++;
			}

		}

		memcpy(index, inputArray, NN * sizeof(inputArray[0]));
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
