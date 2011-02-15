/**
 * This work is licensed under the Creative Commons
 * Attribution-Noncommercial-No Derivative Works 3.0
 * Unported License. To view a copy of this license,
 * visit http://creativecommons.org/licenses/by-nc-nd/3.0/
 * or send a letter to Creative Commons, 171 Second Street,
 * Suite 300, San Francisco, California, 94105, USA.
**/

#ifndef PQSORTH
#define PQSORTH


typedef unsigned int element;




#ifdef _MSC_VER
  #ifdef BUILDING_DLL
    #define DLLEXPORT  __declspec(dllexport)
  #else
    #define DLLEXPORT /*__declspec(dllimport)*/
  #endif
#else
  #ifdef HAVE_GCCVISIBILITYPATCH
    #define DLLEXPORT __attribute__ ((visibility("default")))
  #else
    #define DLLEXPORT
  #endif
#endif



#define MAXTHREADS 256
#define MAXBLOCKS 2048
//#define SBSIZE 1024


typedef struct
{
	unsigned int beg;
	unsigned int end;
	unsigned int orgbeg;
	unsigned int orgend;
	element		 rmaxpiv;
	element		 lmaxpiv;
	element		 rminpiv;
	element		 lminpiv;

	bool		 altered;
	bool		 flip;
	element		 pivot;
} BlockSize;

// Holds parameters to the kernel in phase one
typedef struct
{
	unsigned int from;
	unsigned int end;
	element pivot;
	unsigned int ptr;
	bool last;
} Params;

// Used to perform a cumulative sum between blocks.
// Unnecessary for cards with atomic operations.
// Will be removed when these becomes more common
typedef struct
{
	element maxpiv[MAXBLOCKS];
	element minpiv[MAXBLOCKS];

	unsigned int left[MAXBLOCKS];
	unsigned int right[MAXBLOCKS];
} Length;

// Since we have divided up the kernel in to three
// we need to remember the result of the cumulative sum
// Unnecessary for cards with atomic operations.
// Will be removed when these becomes more common
typedef struct
{
	unsigned int left[(MAXTHREADS)*MAXBLOCKS];
	unsigned int right[(MAXTHREADS)*MAXBLOCKS];
} Hist;

typedef struct
{
	unsigned int beg;
	unsigned int end;
	bool flip;
	unsigned int sbsize;
} LQSortParams;



/**
* The main sort function
* @param data		Data to be sorted
* @param size		The length of the data
* @param timerValue Contains the time it took to sort the data [Optional]
* @returns 0 if successful. For non-zero values, use getErrorStr() for more information about why it failed.
*/

int gpuqsort(element* data, unsigned int size, double* timerValue=0, unsigned int blockscount = 0, unsigned int threads = 0, unsigned int sbsize = 0, unsigned int phase = 0);

// Float support removed due to some problems with CUDA 2.0 and templates
// Will be fixed
//extern "C"
//DLLEXPORT int gpuqsortf(float* data, unsigned int size, double* timerValue=0);

/**
* Returns the latest error message
* @returns the latest error message
*/
//extern "C" DLLEXPORT const char* getGPUSortErrorStr();

#endif
