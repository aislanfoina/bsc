/**
* GPU Quicksort Library
* ---------------------
* Copyright 2007-2008 Daniel Cederman and Philippas Tsigas
*
* This work is licensed under the Creative Commons
* Attribution-Noncommercial-No Derivative Works 3.0
* Unported License. To view a copy of this license,
* visit http://creativecommons.org/licenses/by-nc-nd/3.0/
* or send a letter to Creative Commons, 171 Second Street,
* Suite 300, San Francisco, California, 94105, USA.
**/

#define BUILDING_DLL

#include "stdio.h"
#include "math.h"
#include "string.h"
#include "sscc_gpuqsort.h"

//#include "simpletimer.cu"

#include <algorithm>

// Keep tracks of the data blocks in phase one

#include "gpuqsort_kernels.cu"

#undef THREADS
#define THREADS threads

int err;

bool errCheck(int e) {
/*	if(e==cudaSuccess)
		return true;

	err = e;*/
	return false;
}


element max(element a, element b) {
	if(a>b)
		return a;
	else
		return b;
}

element min(element a, element b) {
	if(a<b)
		return a;
	else
		return b;
}

extern "C" void cuda_part1(element* data, Params* params, Hist* hists, Length* length, int paramsize, int threads);
extern "C" void cuda_part2(element* data, element* data2, Params* params, Hist* hists, Length* length, int paramsize, int threads);
extern "C" void cuda_part3(element* data, Params* params, Hist* hists, Length* length, int paramsize, int threads);
extern "C" void cuda_lqsort(element* data, element* data2, LQSortParams* lqparams, int phase, int worksize, int threads, int sbsize);

#pragma omp target device (cuda) copy_deps
#pragma omp task inout([size]data, [paramsize]params)
void task_part1(element* data, int size, Params* params, Hist* hists, Length* length, int paramsize, int threads) {
	cuda_part1(data, params, hists, length, paramsize, threads);
}

#pragma omp target device (cuda) copy_deps
#pragma omp task inout([size]data, [size]data2, [paramsize]params)
void task_part2(element* data, element* data2, int size, Params* params, Hist* hists, Length* length, int paramsize, int threads) {
	cuda_part2(data, data2, params, hists, length, paramsize, threads);
}

#pragma omp target device (cuda) copy_deps
#pragma omp task inout([size]data, [paramsize]params)
void task_part3(element* data, int size, Params* params, Hist* hists, Length* length, int paramsize, int threads) {
	cuda_part3(data, params, hists, length, paramsize, threads);
}

#pragma omp target device (cuda) copy_deps
#pragma omp task inout([size]data, [size]data2, [2048]lqparams)
void task_lqsort(element* data, element* data2, int size, LQSortParams* lqparams, int phase, int paramsize, int threads, int sbsize) {
	cuda_lqsort(data, data2, lqparams, phase, paramsize, threads, sbsize);
}


/**
* The main sort function
* @param data		Data to be sorted
* @param size		The length of the data
* @param timerValue Contains the time it took to sort the data [Optional]
* @returns 0 if successful. For non-zero values, use getErrorStr() for more information about why it failed.
*/

int gpuqsort(element* data, unsigned int size, double* timerValue, unsigned int blockscount, unsigned int threads, unsigned int sbsize, unsigned int phase)
{

//#pragma omp start

	//Metodos
	element* data2;

	element* ddata;
	element* ddata2;

	Params* params;
//	Params* dparams;

	LQSortParams* lqparams;
//	LQSortParams* dlqparams;

	Hist* dhists;
//	Length* dlength;
	Length* length;
	BlockSize* workset;

	float TK,TM,MK,MM,SM,SK;

	bool init;


	// Construtor


/*	cudaDeviceProp deviceProp;
	cudaGetDeviceProperties(&deviceProp, 0);
	if(!strcmp(deviceProp.name,"GeForce 8800 GTX"))
	{
		TK = 1.17125033316e-005f;
		TM = 52.855721393f;
		MK = 3.7480010661e-005f;
		MM = 476.338308458f;
		SK = 4.68500133262e-005f;
		SM = 211.422885572f;
	}
	else
	if(!strcmp(deviceProp.name,"GeForce 8600 GTS"))
	{
		TK = 0.0f;
		TM = 64.0f;
		MK = 0.0000951623403898f;
		MM = 476.338308458f;
		SK = 0.0000321583081317f;
		SM = 202.666666667f;
	}
	else
	{*/
		TK = 0;
		TM = 128;
		MK = 0;
		MM = 512;
		SK = 0;
		SM = 512;
//	}
/*
	if(cudaMallocHost((void**)&workset,MAXBLOCKS*2*sizeof(BlockSize))!=cudaSuccess) return -1;
	if(cudaMallocHost((void**)&params,MAXBLOCKS*sizeof(Params))!=cudaSuccess) return -1;
	if(cudaMallocHost((void**)&length,sizeof(Length))!=cudaSuccess) return -1;
	if(cudaMallocHost((void**)&lqparams,MAXBLOCKS*sizeof(LQSortParams))!=cudaSuccess) return -1;
*/
	workset = (BlockSize *) malloc(MAXBLOCKS*2*sizeof(BlockSize));
	params = (Params *) malloc(MAXBLOCKS*sizeof(Params));
	length = (Length *) malloc(sizeof(Length));
	lqparams = (LQSortParams *) malloc(MAXBLOCKS*sizeof(LQSortParams));


/*
	if(cudaMalloc((void**)&dlqparams,MAXBLOCKS*sizeof(LQSortParams))!=cudaSuccess) return -1;
	if(cudaMalloc((void**)&dhists,sizeof(Hist))!=cudaSuccess) return -1;
	if(cudaMalloc((void**)&dlength,sizeof(Length))!=cudaSuccess) return -1;
	if(cudaMalloc((void**)&dparams,MAXBLOCKS*sizeof(Params))!=cudaSuccess) return -1;
*/
	init = true;


	// Sort


	if(!init)
		return 1;

	if(!threads||!blockscount||!sbsize)
	{
		threads   = 1<<(int)round(log(size * TK + TM)/log(2.0));
		blockscount = 1<<(int)round(log(size * MK + MM)/log(2.0));
		sbsize    = 1<<(int)round(log(size * SK + SM)/log(2.0));
	}
/*
#ifdef HASATOMICS
		unsigned int* doh;
		unsigned int oh;

		cudaGetSymbolAddress((void**)&doh,"ohtotal");
		oh=0;
		cudaMemcpy(doh,&oh,4,cudaMemcpyHostToDevice);
#endif
*/
	if(threads>MAXTHREADS)
		return 1;

	if(blockscount>MAXBLOCKS)
		return 1;

//	SimpleTimer st;

	// Copy the data to the graphics card and create an auxiallary array
	ddata2 = 0; ddata = 0;

	data2 = (element *) malloc((size)*sizeof(element));
/*
	if(!errCheck(cudaMalloc((void**)&ddata2,(size)*sizeof(element))))
		return 1;
	if(!errCheck(cudaMalloc((void**)&ddata,(size)*sizeof(element))))
		return 1;
	if(!errCheck(cudaMemcpy(ddata, data, size*sizeof(element), cudaMemcpyHostToDevice) ))
		return 1;
*/

/*	if(timerValue!=0)
	{
		// Start measuring time
#pragma omp taskwait
//		cudaThreadSynchronize();

		st.start();
	}
*/
	// We start with a set containg only the sequence to be sorted
	// This will grow as we partition the data
	workset[0].beg = 0;
	workset[0].end = size;
	workset[0].orgbeg = 0;
	workset[0].orgend = size;
	workset[0].altered = false;
	workset[0].flip = false;

	// Get a starting pivot
	workset[0].pivot = (min(min(data[0],data[size/2]),data[size-1]) + max(max(data[0],data[size/2]),data[size-1]))/2;
	unsigned int worksize = 1;

	unsigned int blocks = blockscount/2;
	unsigned totsize = size;
	unsigned int maxlength = (size/blocks)/4;

	unsigned int iterations = 0;
	bool flip = true;

	// Partition the sequences until we have enough
	while(worksize<blocks)
	{
		unsigned int ws = totsize/blocks;
		unsigned int paramsize = 0;

		// Go through the sequences we have and divide them into sections
		// and assign thread blocks according to their size
		for(unsigned int i=0;i<worksize;i++)
		{
			if((workset[i].end-workset[i].beg)<maxlength)
				continue;

			// Larger sequences gets more thread blocks assigned to them
			unsigned int blocksassigned = max((workset[i].end-workset[i].beg)/ws,1);
			for(unsigned int q=0;q<blocksassigned;q++)
			{
				params[paramsize].from = workset[i].beg + ws*q;
				params[paramsize].end = params[paramsize].from + ws;
				params[paramsize].pivot = workset[i].pivot;
				params[paramsize].ptr = i;
				params[paramsize].last = false;
				paramsize++;

			}
			params[paramsize-1].last = true;
			params[paramsize-1].end = workset[i].end;

			workset[i].lmaxpiv=0;
			workset[i].lminpiv=0xffffffff;
			workset[i].rmaxpiv=0;
			workset[i].rminpiv=0xffffffff;
		}

		if(paramsize==0)
			break;
/*
		// Copy the block assignment to the GPU
		if(!errCheck(cudaMemcpy(dparams, params, paramsize*sizeof(Params), cudaMemcpyHostToDevice) ))
			return 1;
*/
		// Do the cumulative sum
		if(flip)
			task_part1(data, size, params, dhists, length, paramsize, THREADS);
		else
			task_part1(data2, size, params, dhists, length, paramsize, THREADS);
/*
		if(!errCheck((cudaMemcpy(length, dlength,sizeof(Length) , cudaMemcpyDeviceToHost) )))
			return 1;
*/
		// Do the block cumulative sum. Done on the CPU since not all cards have support for
		// atomic operations yet.
		for(unsigned int i=0;i<paramsize;i++)
		{
			unsigned int l = length->left[i];
			unsigned int r = length->right[i];

			length->left[i] = workset[params[i].ptr].beg;
			length->right[i] = workset[params[i].ptr].end;

			workset[params[i].ptr].beg+=l;
			workset[params[i].ptr].end-=r;
			workset[params[i].ptr].altered = true;

			workset[params[i].ptr].rmaxpiv = max(length->maxpiv[i],workset[params[i].ptr].rmaxpiv);
			workset[params[i].ptr].lminpiv = min(length->minpiv[i],workset[params[i].ptr].lminpiv);

			workset[params[i].ptr].lmaxpiv = min(workset[params[i].ptr].pivot,workset[params[i].ptr].rmaxpiv);
			workset[params[i].ptr].rminpiv = max(workset[params[i].ptr].pivot,workset[params[i].ptr].lminpiv);


		}
/*
		// Copy the result of the block cumulative sum to the GPU
		if(!errCheck((cudaMemcpy(dlength, length, sizeof(Length), cudaMemcpyHostToDevice) )))
			return 1;
*/
		// Move the elements to their correct position
		if(flip)
			task_part2(data, data2, size, params, dhists, length, paramsize, THREADS);
		else
			task_part2(data2, data, size, params, dhists, length, paramsize, THREADS);

		// Fill in the pivot value between the left and right blocks
		task_part3(data, size, params, dhists, length, paramsize, THREADS);

		flip = !flip;

		// Add the sequences resulting from the partitioning
		// to set
		unsigned int oldworksize = worksize;
		totsize = 0;
		for(unsigned int i=0;i<oldworksize;i++)
		{
			if(workset[i].altered)
			{
				if(workset[i].beg-workset[i].orgbeg>=maxlength)
					totsize += workset[i].beg-workset[i].orgbeg;
				if(workset[i].orgend-workset[i].end>=maxlength)
					totsize += workset[i].orgend-workset[i].end;

				workset[worksize].beg = workset[worksize].orgbeg = workset[i].orgbeg;
				workset[worksize].end = workset[worksize].orgend = workset[i].beg;
				workset[worksize].flip=flip;
				workset[worksize].altered = false;
				workset[worksize].pivot = (workset[i].lminpiv/2+workset[i].lmaxpiv/2);

				worksize++;

				workset[i].orgbeg = workset[i].beg = workset[i].end;
				workset[i].end = workset[i].orgend;
				workset[i].flip=flip;
				workset[i].pivot = (workset[i].rminpiv/2+workset[i].rmaxpiv/2);
				workset[i].altered = false;
			}
		}
		iterations++;

	}

	// Due to the poor scheduler on some graphics card
	// we need to sort the order in which the blocks
	// are sorted to avoid poor scheduling decisions
	unsigned int sortblocks[MAXBLOCKS*2];
	for(int i=0;i<worksize;i++)
		sortblocks[i]=((workset[i].end-workset[i].beg)<<(int)round(log((float)(MAXBLOCKS*4.0f))/log(2.0f))) + i;
	std::sort(&sortblocks[0],&sortblocks[worksize]);

	if(worksize!=0)
	{
		// Copy the block assignments to the GPU
		for(int i=0;i<worksize;i++)
		{
		 	unsigned int q = (worksize-1)-(sortblocks[i]&(MAXBLOCKS*4-1));

			lqparams[i].beg =  workset[q].beg;
			lqparams[i].end = workset[q].end;
			lqparams[i].flip = workset[q].flip;
			lqparams[i].sbsize = sbsize;
		}
/*
		if(!errCheck((cudaMemcpy(dlqparams, lqparams, worksize*sizeof(LQSortParams), cudaMemcpyHostToDevice) )))
			return 1;
*/

		// Run the local quicksort, the one that doesn't need inter-block synchronization
		if(phase!=1)
			task_lqsort(data, data2, size, lqparams, phase, worksize, THREADS, sbsize);
	}

#pragma omp taskwait
/*
	if(timerValue!=0)
	{
		// Measure the time taken
		*timerValue = st.end();
	}
*/
#pragma omp taskwait

	// Free the data
/*	if(err!=cudaSuccess)
	{
		cudaFree(ddata);
		cudaFree(ddata2);
		return 1;
	}*/

	// Copy the result back to the CPU
/*	if(!errCheck((cudaMemcpy(data, ddata, size*sizeof(element), cudaMemcpyDeviceToHost) )))
		return 1;

	cudaFree(ddata);
	cudaFree(ddata2);
*/


	// Destrutor
/*
	cudaFreeHost(workset);
	cudaFreeHost(params);
	cudaFreeHost(length);
	cudaFreeHost(lqparams);
	cudaFree(dparams);
	cudaFree(dlqparams);
	cudaFree(dhists);
	cudaFree(dlength);
*/

	return 0;
}

/**
* Fills data with elements from a specific distribution
* @param data List to hold the data
* @param size The size of the list
* @param type The type of distribution to pick elements from
*/
void dist(element* data, unsigned int size, int type)
{
	#define MAXVAL 0xffffffe

//	element temp;
//	unsigned int numThreads = 128;

	for(unsigned int i=0;i<size;i++)
		data[i]=(rand() + i)%1000000;
}




int main() {
	const unsigned int ITERATIONS = 1;
	const unsigned int MEASURES = 1;
	const unsigned int DISTRIBUTIONS = 1;
	const unsigned int STARTSIZE = 2<<19;

	// Allocate memory for the sequences to be sorted
	unsigned int maxsize = STARTSIZE<<(MEASURES-1);
	element* data = new element[maxsize];
	element* data2 = new element[maxsize];

	double timerValue;
	unsigned int run = 0;

	// Go through all distributions
	for(int d=0;d<DISTRIBUTIONS;d++)
	{
		unsigned int testsize = STARTSIZE;

		// Go through all sizes
		for(int i=0;i<MEASURES;i++,testsize<<=1)
		{
			// Do it several times
			for(int q=0;q<ITERATIONS;q++)
			{
				// Create sequence according to distribution
				dist(data,testsize,d);
				// Store copy of sequence
				memcpy(data2,data,testsize*sizeof(element));

				int threads  =0;
				int maxblocks=0;
				int sbsize   =0;

				printf("Data: \n");
				for(int z=0; z < testsize; z+=1000)
					printf("%d ",data[z]);
				printf("\n\n****************************\n\n\n");

				// Sort it
				if(gpuqsort(data,testsize,&timerValue,maxblocks,threads,sbsize,0)!=0)
				{
					printf("Error!\n");
					exit(1);
				}

				printf("Data: \n");
				for(int z=0; z < testsize; z+=1000)
					printf("%d ",data[z]);
				printf("\n");


				// Validate the result
				printf("%d/%d!\n",run++,MEASURES*DISTRIBUTIONS*ITERATIONS);
			}
		}
	}

}




// Float support removed due to some problems with CUDA 2.0 and templates
// Will be fixed

//extern "C" DLLEXPORT
/*int gpuqsortf(float* data, unsigned int size, double* timerValue)
{
	GPUQSort<float> s;
	if(s.sort(data,size,timerValue)!=0)
	{
		expErrMsg = (char*)s.getErrorStr();
		return 1;
	}
	else
		return 0;
}*/

