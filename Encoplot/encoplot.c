#include <sys/time.h>
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "math.h"

#define insidetasks

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

word_t readat(unsigned char *buf,int poz)
{
	int depth=sizeof(word_t);
	word_t rez=0;
	int i;
	for(i = 0; i < depth; i++){rez<<=8;rez|=buf[poz+depth-1-i];}
	return rez;
}

int compare_number (const word_t *a, const word_t *b) {
	if (*a > *b)
		return 1;
	else
		return -1;

	return 0;
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

//	ELM *array;
//	array = malloc(NN * sizeof(ELM));

/*	ELM *array2;
	array2 = malloc(NN * sizeof(ELM));

	word_t *array3;
	array3 = malloc(NN * sizeof(word_t));*/

	for (i = 0; i < NN; i += block_records) {
		chunk_of_records = (i + block_records > NN ? NN - i : block_records);
		init_ELMArray_task(&array[i], &buffer[i], chunk_of_records, DEPTH, i);
	 }
/*
	for (i = 0; i < NN; i++) {
		array[i].ngram = readat(buffer, i);
		array[i].index = i;

		array2[i].ngram = readat(buffer, i);
		array2[i].index = i;

		array3[i] = array[i].ngram;
	}*/

//	psort(array2, NN);
//	qsort(array2, NN, sizeof(ELM),compareELM);

#pragma css barrier

	pngramsort(array, NN);
/*
	for (i = 0; i < NN; i++) {
		index[i] = array[i].index;
	}*/
	for (i = 0; i < NN; i += block_records) {
		chunk_of_records = (i + block_records > NN ? NN - i : block_records);
		copy_ELMArray_task(&array[i], &index[i], chunk_of_records);
	 }

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



void simpler_rsort4ngrams(unsigned char *buffer, int numlines, int DEPTH, int *index, int *counter_output, int *startpos_output) {
	const int RANGE = 256;

	int a, b, c;
	int i, j, k;

//	int big_block_records = 60;
	int big_block_records = 20000000;
	int block_records = 500000000;
	int chunk_of_records = 0;

	if(numlines > big_block_records+DEPTH) {
		int pages = (int)ceil((float)numlines/(float)big_block_records);
		t_int **index_pages;
		t_int **counter_pages;
		t_int **startpos_pages;

		index_pages = malloc(pages*sizeof(t_int*));
		counter_pages = malloc(pages*sizeof(t_int*));
		startpos_pages = malloc(pages*sizeof(t_int*));
		b = 0;
		for (a = 0; a < numlines; a += big_block_records) {
			int chunk_of_records = (a + big_block_records > numlines ? numlines - a : big_block_records + DEPTH - 1);
			index_pages[b] = (t_int*) malloc(chunk_of_records * sizeof(t_int));
			counter_pages[b] = (t_int*) malloc(RANGE * sizeof(t_int));
			startpos_pages[b] = (t_int*) malloc(RANGE * sizeof(t_int));

			simpler_rsort4ngrams(&buffer[a], chunk_of_records, DEPTH, index_pages[b], counter_pages[b], startpos_pages[b]);

			if(a > 0) {
				for(c = 0; c < chunk_of_records; c++)
					index_pages[b][c] += a;
			}
			b++;
		}
//		printf("Merge\n");
		int NN = numlines - DEPTH + 1;
		t_int *inputArray = (t_int*) malloc(NN * sizeof(t_int));

		t_int counters[RANGE];
		t_int startpos[RANGE];

		//task2 - independent - input RANGE inout counters
		init_counters_task(counters, RANGE, 0);

		//task3 histogram - reduction - input NN, buffer inout counters

		for (k = 0; k < NN; k += block_records) {
			chunk_of_records = (k + block_records > NN ? NN - k : block_records);
			counters_load_task(counters, RANGE, buffer, numlines, k+chunk_of_records, k);
		}

		unsigned char *pin = buffer + NN;
		unsigned char *pout = buffer;
		for (j = 0; j < DEPTH - 1; j++) {
			counters[*pout++]--;
			counters[*pin++]++;
		}

		//task 4
		init_startpos_task(startpos, counters, RANGE);

		for (i = 0; i < RANGE; i++) {
			int offset = 0;
			for (j = 0; j < pages; j++) {
				if(counter_pages[j][i] > 0) {
//					printf("i = %d, j = %d, startpos[i] = %d, offset = %d\n", i, j, startpos[i], offset);
//					printf("startpos_pages[j][i] = %d, index_pages[j][startpos_pages[j][i]] = %d, counter_pages[j][i] = %d\n",startpos_pages[j][i],index_pages[j][startpos_pages[j][i]], counter_pages[j][i]);
//					printf("startpos[i+1] = %d, startpos_pages[j][i+1] = %d, counter_pages[j][i+1] = %d\n", startpos[i+1], startpos_pages[j][i+1], counter_pages[j][i+1]);
				}
				memcpy_task(&index[startpos[i]+offset], &index_pages[j][startpos_pages[j][i]], counter_pages[j][i], sizeof(t_int));
				if(counter_pages[j][i] > 0) {
//					printf("Copied!\n");
				}
				offset += counter_pages[j][i];
			}
		}
		return;
	}
	else {

	int NN = numlines - DEPTH + 1;

	if (NN > 0) {
		unsigned char *pin = buffer + NN;
		unsigned char *pout = buffer;
		typedef int t_int;
		t_int *inputArray = (t_int*) malloc(NN * sizeof(t_int));
		t_int *outputArray = (t_int*) malloc(NN * sizeof(t_int));
		t_int counters[RANGE];
		t_int startpos[RANGE];







		//task1 - independent - input NN inout inputArray
		  for (k = 0; k < NN; k += block_records) {
  			chunk_of_records = (k + block_records > NN ? NN - k : block_records);
			init_inputArray_task(&inputArray[k], chunk_of_records, k);
		  }


		//radix sort, the input is x, the output rank is ix
		//counters


		//task2 - independent - input RANGE inout counters
		init_counters_task(counters, RANGE, 0);


		//task3 histogram - reduction - input NN, buffer inout counters

		for (k = 0; k < NN; k += block_records) {
			chunk_of_records = (k + block_records > NN ? NN - k : block_records);
			counters_load_task(counters, RANGE, buffer, numlines, k+chunk_of_records, k);
		}

		for(j=0; j < DEPTH; j++) {
			int ofs = j;//low endian
			t_int sp = 0;


			//task 4
			init_startpos_task(startpos, counters, RANGE);

			//task 5 major loop - task reduction

#pragma css barrier

			for (i = 0; i < RANGE; i++) {
				for(k = 0; k < counters[i]; k+= block_records) {
					chunk_of_records = (k + block_records > counters[i] ? counters[i] - k : block_records);
					load_outputArray_task(inputArray, &outputArray[startpos[i]+k], buffer, numlines, NN, ofs, k, i, chunk_of_records);
				}
			}

#pragma css barrier

			for (k = 0; k < NN; k += block_records) {
				chunk_of_records = (k + block_records > NN ? NN - k : block_records);
				memcpy_task(&inputArray[k], &outputArray[k], chunk_of_records, sizeof(inputArray[0]));
			}

#pragma css barrier

			//update counters
			if (j < DEPTH - 1) {
				counters[*pout++]--;
				counters[*pin++]++;
			}

		}

#pragma css wait on (inputArray)

		memcpy_task(counter_output, counters, RANGE, sizeof(t_int));
		memcpy_task(startpos_output, startpos, RANGE, sizeof(t_int));

		for (k = 0; k < NN; k += block_records) {
			chunk_of_records = (k + block_records > NN ? NN - k : block_records);
			memcpy_task(&index[k], &inputArray[k], chunk_of_records, sizeof(inputArray[0]));
		}

		free(outputArray);

	} else
		index = NULL;
	}
}

#define MAXBUFSIZ 8000123
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
//void startidx(ELM *array, int size, t_int *index, int num_idx) {
void startidx(int size, t_int *index, int num_idx) {
	int i;
	int num = 0;

	int block_records = BLOCK_RECORDS;

	for (i = 0; i < size; i+= block_records) {
		index[num] = (t_int) i;
		num++;
	}
//	*num_idx = num;
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

//	int linecnt1 = idx_ar1;
//	int linecnt2 = idx_ar2;

	tmp1 = &array1[idx_ar1];
	tmp2 = &array2[idx_ar2];

	ar1Last = &array1[numlines1-1];
	ar2Last = &array2[numlines2-1];

	while (startVal >= tmp2->ngram) {
		tmp2++;
//		linecnt2++;
//		tmp2 = array2[linecnt2];
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
	/*
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
*/
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
//			compareArray(&array1[index_ar1[cnt1]], &array2[index_ar2[cnt2-1]], startVal, endVal, &cnt[cnt1]);
		}
	}
	int total = 0;
	for(i = 0; i < cnt1; i++)
		total += cnt[i];
	printf("++++ cnt = %d\n",total);
}


/*
inline word_t readat(const unsigned char *buf, int poz) {
	return *(word_t *) (buf + poz);
}
*/

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
	t_int *counter1 = (t_int*) malloc(256 * sizeof(t_int));
	t_int *startpos1 = (t_int*) malloc(256 * sizeof(t_int));
	t_int *index2 = (t_int*) malloc(numlines2 * sizeof(t_int));
	t_int *counter2 = (t_int*) malloc(256 * sizeof(t_int));
	t_int *startpos2 = (t_int*) malloc(256 * sizeof(t_int));

	maintime_int(0);
	printf("Original\n");

	simpler_rsort4ngrams_orig(bufferfile1, numlines1, depth, index1);
	simpler_rsort4ngrams_orig(bufferfile2, numlines2, depth, index2);

#pragma css barrier

	maintime_int(1);
	printf("Novo\n");

	simpler_qsort4ngrams(bufferfile1, numlines1, depth, index1, array1);
	simpler_qsort4ngrams(bufferfile2, numlines2, depth, index2, array2);
//	simpler_rsort4ngrams(bufferfile1, numlines1, depth, index1, counter1, startpos1);
//	simpler_rsort4ngrams(bufferfile2, numlines2, depth, index2, counter2, startpos2);

#pragma css barrier

	maintime_int(1);
printf("Comparacao Nova\n");

	compare2files(array1, numlines1, array2, numlines2, depth);
#pragma css barrier

	maintime_int(1);
	printf("Comparacao Antiga\n");

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
