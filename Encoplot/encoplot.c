#include <sys/time.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "math.h"

#define BENCHMARK

#define BLOCK_RECORDS 500000

typedef int t_int;

//typedef int word_t;
//typedef __uint128_t word_t;
typedef __uint64_t word_t;
//typedef __uint32_t word_t;

//CG rsort
#define fr(x,y)for(int x=0;x<y;x++)

typedef struct ELM_t
{
	word_t ngram;
	t_int index;
} ELM;


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


#pragma css task input (SIZE, typeSize, orig[SIZE]) \
                 output (dst[SIZE])

void memcpy_task(t_int *dst, t_int *orig, int SIZE, int typeSize){
	memcpy(dst, orig, SIZE * typeSize);
}

word_t readat(unsigned char *buf,int poz)
{
	int depth=sizeof(word_t);
	word_t rez=0;
	int i;
	for(i = 0; i < depth; i++){rez<<=8;rez|=buf[poz+depth-1-i];}
	return rez;
}

int compareELM (const ELM *a, const ELM *b) {
	if(a->ngram < b->ngram)
		return -1;
	else if (a->ngram > b->ngram)
		return 1;
	else {
//		return 0;
		if(a->index < b->index)
			return -1;
		else
			return 1;
	}
}


#pragma css task input (size, offset, depth, buffer[size+depth]) \
                 output (array[size])
void init_ELMArray_task(ELM *array, unsigned char *buffer, int size, int depth, int offset) {
	int i;
	for(i=0;i < size; i++) {
		array[i].ngram = readat(buffer, i);
		array[i].index = i+offset;
	}
}


#pragma css task input (size, array[size]) \
                 output (index[size])
void copy_ELMArray_task(ELM *array, t_int *index, int size) {
	int i;
	for(i=0;i < size; i++) {
		index[i] = array[i].index;
	}
}

void simpler_qsort4ngrams(unsigned char *buffer, int numlines, int DEPTH, t_int *index, ELM *array) {
	int block_records = 500000;

	int chunk_of_records;
	int NN = numlines - DEPTH + 1;
	int i;

	for (i = 0; i < NN; i += block_records) {
		chunk_of_records = (i + block_records > NN ? NN - i : block_records);
		init_ELMArray_task(&array[i], &buffer[i], chunk_of_records, DEPTH, i);
	 }

#pragma css barrier

	pngramsort(array, NN);

// For compatibility porpouse...

/*	for (i = 0; i < NN; i += block_records) {
		chunk_of_records = (i + block_records > NN ? NN - i : block_records);
		copy_ELMArray_task(&array[i], &index[i], chunk_of_records);
	 }
*/

}


void simpler_rsort4ngrams_orig(unsigned char *buffer, int numlines, int DEPTH, int *index) {
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


#define MAXBUFSIZ 1000012300

unsigned char bufferfile1[MAXBUFSIZ];
unsigned char bufferfile2[MAXBUFSIZ];
int numlines1, numlines2;


int isELMequal (ELM *a, ELM *b) {
	if(a->ngram == b->ngram && a->index == b->index)
		return 1;
	else
		return 0;
}

#pragma css task input (size, num_idx) \
                 output (index[num_idx])

void startidx(int size, t_int *index, int num_idx) {
	int i;
	int num = 0;

	int block_records = BLOCK_RECORDS;

	for (i = 0; i < size; i+= block_records) {
		index[num] = (t_int) i;
		num++;
	}
}

#pragma css task input (numlines1, numlines2, idx_ar1, idx_ar2, array1[numlines1], array2[numlines2], startVal, endVal) \
                 output (cnt)
void compareArray(ELM *array1, int numlines1, t_int idx_ar1, ELM *array2, int numlines2, t_int idx_ar2, word_t startVal, word_t endVal, int *cnt) {
	int i;
	int tmp_cnt = 0;

	ELM *tmp1;
	ELM *tmp2;

	ELM *ar1Last;
	ELM *ar2Last;

	tmp1 = &array1[idx_ar1];
	tmp2 = &array2[idx_ar2];

	ar1Last = &array1[numlines1-1];
	ar2Last = &array2[numlines2-1];

	if(!isELMequal(tmp1,&array1[0])) {
		while (startVal >= tmp2->ngram && !isELMequal(tmp2,ar2Last)) {
			tmp2++;
		}
	}

	int stop = 0;

	while (tmp1->ngram <= endVal && !stop) { // && !isELMequal(tmp1, ar1Last) && !isELMequal(tmp2,ar2Last)) {
		if (tmp1->ngram == tmp2->ngram) {
//				printf("%d %d\n", index1[linecnt1], index2[linecnt2]);
			tmp_cnt++;
			if(!isELMequal(tmp1, ar1Last)) {
				tmp1++;
			}
			else
				stop = 1;
			if(!isELMequal(tmp2, ar2Last)) {
				tmp2++;
			}
			else
				stop = 1;
		}
		else if (tmp1->ngram < tmp2->ngram) {
			if(!isELMequal(tmp1, ar1Last)) {
				tmp1++;
			}
			else
				stop = 1;

		}
		else if (tmp1->ngram > tmp2->ngram) {
			if(!isELMequal(tmp2, ar2Last)) {
				tmp2++;
			}
			else
				stop = 1;
		}
	}
	*cnt = tmp_cnt;
}


void compare2files(ELM *array1, int numlines1, ELM *array2, int numlines2, int depth) {
	int block_records = BLOCK_RECORDS;

	numlines1 -= depth-1;
	numlines2 -= depth-1;

	int chunk_of_records;
	int i;

	int num_idx1 = ceil((float) numlines1 / (float) block_records);
	int num_idx2 = ceil((float) numlines2 / (float) block_records);

	t_int *index_ar1 = (t_int *) malloc (num_idx1 * sizeof(t_int));
	t_int *index_ar2 = (t_int *) malloc (num_idx2 * sizeof(t_int));

	startidx(numlines1, index_ar1, num_idx1);
	startidx(numlines2, index_ar2, num_idx2);

	int *cnt = malloc (num_idx1 * sizeof(int));
	memset(cnt, 0, sizeof(cnt));

	int cnt1 = 0, cnt2 = 0;

#pragma css barrier

	for(cnt1 = 0; cnt1 < num_idx1; cnt1++) {
		ELM *tmp1;
		tmp1 = &array1[index_ar1[cnt1]];

		ELM *tmp2;
		tmp2 = &array2[index_ar2[cnt2]];

		while(tmp1->ngram > tmp2->ngram && cnt2 < num_idx2-1) {
			cnt2++;
			tmp2 = &array2[index_ar2[cnt2]];
		}
		word_t startVal = tmp1->ngram;
		word_t endVal;

		if (cnt1+1 == num_idx1) {
			ELM *tmp1plus;
			tmp1plus = &array1[numlines1-1];
			endVal = tmp1plus->ngram;
		}
		else {
			ELM *tmp1plus;
			tmp1plus = &array1[index_ar1[cnt1+1]];
			endVal = tmp1plus->ngram;
		}

		if(cnt2-1 < 0) {
			compareArray(array1,numlines1,index_ar1[cnt1], array2, numlines2, index_ar2[cnt2], startVal, endVal, &cnt[cnt1]);
		}
		else {
			compareArray(array1,numlines1,index_ar1[cnt1], array2, numlines2, index_ar2[cnt2-1], startVal, endVal, &cnt[cnt1]);
		}
	}

#pragma css barrier

	int total = 0;
	for(i = 0; i < cnt1; i++)
		total += cnt[i];
	printf("++++ cnt = %d\n",total);
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



#pragma css start

	//index the ngrams
	ELM *array1 = (ELM *) malloc(numlines1 * sizeof(ELM));
	ELM *array2 = (ELM *) malloc(numlines1 * sizeof(ELM));


	t_int *index1 = (t_int*) malloc(numlines1 * sizeof(t_int));
	t_int *index2 = (t_int*) malloc(numlines2 * sizeof(t_int));

	maintime_int(0);
	printf("New sort\n");

	simpler_qsort4ngrams(bufferfile1, numlines1, depth, index1, array1);
	simpler_qsort4ngrams(bufferfile2, numlines2, depth, index2, array2);

#pragma css barrier

	maintime_int(1);
	printf("New comparing\n");

	compare2files(array1, numlines1, array2, numlines2, depth);
#pragma css barrier

#ifdef BENCHMARK
	maintime_int(1);
	printf("Original sort\n");

	simpler_rsort4ngrams_orig(bufferfile1, numlines1, depth, index1);
	simpler_rsort4ngrams_orig(bufferfile2, numlines2, depth, index2);

#pragma css barrier

	maintime_int(1);
	printf("Original comparing\n");

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
//		free(index2);
//		free(index1);
	}
	printf(">>>> cnt = %d\n\n", cnt);
#endif
	maintime_int(1);
#pragma css finish
	return 0;
}
