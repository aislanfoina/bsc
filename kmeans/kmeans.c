//#define CELL	//Define it if the code will be compiled using the CellSs
#ifdef CELL
#include <spu_intrinsics.h>
#endif

#include <sys/time.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


//#define REPORT	// Show report in the end of the code.
//#define DEBUG	// Debug messages
//#define PAUSE	// Stop during the iterations

/*
 * When the ratio of record movement across custers (led by centroids) hits
 * this low bound, the algorithms does not perform any more iterations.
 */
// Aislan: Threshold reduced from 0.01f to 0.001f

#define TERMINATION_THRESHOLD  0.001f

/*
 * Iteration hard limit.
 */
// Aislan: Max iterations increased from 100 to 1000

#define MAX_ITERATIONS         1000

/*
 * Size hint for the record block.
 */

#define BLOCK_SIZE             8192

void pVector(char *label,int count, float *vec, int DIM) {
	int i;
	if (count != -1) {
		printf("Vec[%s%d] = (",label,count);
	}
	else {
		printf("Vec[%s] = (",label);
	}
	for (i=0; i<DIM-1; i++)
		printf("%9.3f, ",vec[i]);
	printf("%9.3f) ",vec[i]);
}

/*********************************            *********************************/
/*********************************  SPU CODE  *********************************/
/*********************************            *********************************/


/******************************************************************************
 *                                                                            *
 * Find the nearest centroid for a record.  Returns the center identifier     *
 * (index).                                                                   *
 *                                                                            *
 * This is a straightforward adaptation of the original function              *
 *                                                                            *
 ******************************************************************************/
int compare_to_centers(float *record, int dimension, int number_of_centers,
		float *centers) {

	short k;
	short i, j;
	short min_id = 0;
	float min = 1e18;
	short blocksize = 12;

#ifdef CELL

	short loopcount = dimension / blocksize;
	short extra = dimension % blocksize;

#else

	short loopcount = 0;
	short extra = dimension;

#endif

	float *recordsPointer = record;

	// Creation of 4 groups of variables to do 4 calculations in one loop.

	float *center1 = centers;
	float distance;
	float distance_3 = 0.0;
	float distance_4 = 0.0;
	float distance_5 = 0.0;

	int offsetX = dimension;
	float *center2 = centers + offsetX;
	float Cdistance;
	float Cdistance_3 = 0.0;
	float Cdistance_4 = 0.0;
	float Cdistance_5 = 0.0;

	offsetX += dimension;
	float *center3 = centers + offsetX;
	float Ddistance;
	float Ddistance_3 = 0.0;
	float Ddistance_4 = 0.0;
	float Ddistance_5 = 0.0;

	offsetX += dimension;
	float *center4 = centers + offsetX;
	float Edistance;
	float Edistance_3 = 0.0;
	float Edistance_4 = 0.0;
	float Edistance_5 = 0.0;

	//for each 4 centers.
	//Aislan: Removed the -4 sub in the condition of the loop.



	for (k = 0; k < number_of_centers; k += 4) {


		distance_3 = 0.0;
		distance_4 = 0.0;
		distance_5 = 0.0;
#ifdef CELL
		vector float* varray1 = (vector float*)(recordsPointer);
		vector float* varray2 = (vector float*)(center1);
		vector float result1;
		vector float result2;
		vector float result3;
#endif
		Cdistance_3 = 0.0;
		Cdistance_4 = 0.0;
		Cdistance_5 = 0.0;
#ifdef CELL
		vector float* Cvarray2 = (vector float*)(center2);
		vector float Cresult1;
		vector float Cresult2;
		vector float Cresult3;
#endif
		Ddistance_3 = 0.0;
		Ddistance_4 = 0.0;
		Ddistance_5 = 0.0;
#ifdef CELL
		vector float* Dvarray2 = (vector float*)(center3);
		vector float Dresult1;
		vector float Dresult2;
		vector float Dresult3;
#endif
		Edistance_3 = 0.0;
		Edistance_4 = 0.0;
		Edistance_5 = 0.0;
#ifdef CELL
		vector float* Evarray2 = (vector float*)(center4);
		vector float Eresult1;
		vector float Eresult2;
		vector float Eresult3;
		vector float temp,Ctemp,Dtemp,Etemp;
#endif
		int index = 0;
#ifdef CELL
		//accumulators
		vector float C1acc = {0,0,0,0};
		vector float C2acc = {0,0,0,0};
		vector float C3acc = {0,0,0,0};
		vector float C4acc = {0,0,0,0};

		for (i = 0; i < loopcount; i++) { // For each 12 dimensions, one loop of this is executed.

			// Subtraction of the record with the 4 centers.
			// Each 12 dimension vector is splited in a 3 vectors of 4 dimensions.

			result1 = spu_sub(varray1[index], varray2[index]);
			result2 = spu_sub(varray1[index + 1], varray2[index + 1]);
			result3 = spu_sub(varray1[index + 2], varray2[index + 2]);
			Cresult1 = spu_sub(varray1[index], Cvarray2[index]);
			Cresult2 = spu_sub(varray1[index + 1], Cvarray2[index + 1]);
			Cresult3 = spu_sub(varray1[index + 2], Cvarray2[index + 2]);
			Dresult1 = spu_sub(varray1[index], Dvarray2[index]);
			Dresult2 = spu_sub(varray1[index + 1], Dvarray2[index + 1]);
			Dresult3 = spu_sub(varray1[index + 2], Dvarray2[index + 2]);
			Eresult1 = spu_sub(varray1[index], Evarray2[index]);
			Eresult2 = spu_sub(varray1[index + 1], Evarray2[index + 1]);
			Eresult3 = spu_sub(varray1[index + 2], Evarray2[index + 2]);

			// TODO: Analyze these instructions.
			// these can be SIMD'd
			//just keep in vector accumulators until the looping ends

/*			 C1acc = spu_madd(result1,result1,C1acc);
			 C2acc = spu_madd(Cresult1,Cresult1,C2acc);
			 C3acc = spu_madd(Dresult1,Dresult1,C3acc);
			 C4acc = spu_madd(Eresult1,Eresult1,C4acc);

			 C1acc = spu_madd(result2,result2,C1acc);
			 C2acc = spu_madd(Cresult2,Cresult2,C2acc);
			 C3acc = spu_madd(Dresult2,Dresult2,C3acc);
			 C4acc = spu_madd(Eresult2,Eresult2,C4acc);

			 C1acc = spu_madd(result3,result3,C1acc);
			 C2acc = spu_madd(Cresult3,Cresult3,C2acc);
			 C3acc = spu_madd(Dresult3,Dresult3,C3acc);
			 C4acc = spu_madd(Eresult3,Eresult3,C4acc);*/

			// Squaring the results of each subtraction.

			result1 = spu_mul(result1, result1);
			result2 = spu_mul(result2, result2);
			result3 = spu_mul(result3, result3);
			Cresult1 = spu_mul(Cresult1, Cresult1);
			Cresult2 = spu_mul(Cresult2, Cresult2);
			Cresult3 = spu_mul(Cresult3, Cresult3);
			Dresult1 = spu_mul(Dresult1, Dresult1);
			Dresult2 = spu_mul(Dresult2, Dresult2);
			Dresult3 = spu_mul(Dresult3, Dresult3);
			Eresult1 = spu_mul(Eresult1, Eresult1);
			Eresult2 = spu_mul(Eresult2, Eresult2);
			Eresult3 = spu_mul(Eresult3, Eresult3);

			// Summing the 3 distance results vectors of each center into one.

			C1acc = spu_add(result1, result2);
			C2acc = spu_add(Cresult1, Cresult2);
			C3acc = spu_add(Dresult1, Dresult2);
			C4acc = spu_add(Eresult1, Eresult2);

			C1acc = spu_add(C1acc, result3);
			C2acc = spu_add(C2acc, Cresult3);
			C3acc = spu_add(C3acc, Dresult3);
			C4acc = spu_add(C4acc, Eresult3);

			index = index + 3;
		}

		// Converting the vector into a scalar float containing the total distance.

		distance = spu_extract(C1acc, 0) + spu_extract(C1acc, 1)
				+ spu_extract(C1acc, 2) + spu_extract(C1acc, 3);
		Cdistance = spu_extract(C2acc, 0) + spu_extract(C2acc, 1)
				+ spu_extract(C2acc, 2) + spu_extract(C2acc, 3);
		Ddistance = spu_extract(C3acc, 0) + spu_extract(C3acc, 1)
				+ spu_extract(C3acc, 2) + spu_extract(C3acc, 3);
		Edistance = spu_extract(C4acc, 0) + spu_extract(C4acc, 1)
				+ spu_extract(C4acc, 2) + spu_extract(C4acc, 3);



		if (__builtin_expect((extra > 0), 0)) { // For each remaining dimension, do one loop of this.
#else
			distance = 0;
			Cdistance = 0;
			Ddistance = 0;
			Edistance = 0;
#endif
			for (i = 0; i < extra; i++) { // TODO: SIMD it.
				float distance1 = recordsPointer[index + i]	- center1[index + i];
				distance += distance1 * distance1;
				float Cdistance1 = recordsPointer[index + i] - center2[index + i];
				Cdistance += Cdistance1 * Cdistance1;
				float Ddistance1 = recordsPointer[index + i] - center3[index + i];
				Ddistance += Ddistance1 * Ddistance1;
				float Edistance1 = recordsPointer[index + i] - center4[index + i];
				Edistance += Edistance1 * Edistance1;

#ifdef DEBUG
		printf("  *distance[id %d] = %f \n",k,distance);
	    printf("  *Cdistance[id %d] = %f \n",k+1,Cdistance);
	    printf("  *Ddistance[id %d] = %f \n",k+2,Ddistance);
	    printf("  *Edistance[id %d] = %f \n",k+3,Edistance);
#endif

			}
#ifdef CELL
		}
#endif

#ifdef DEBUG
		printf("distance[id %d] = %f \n",k,distance);
	    printf("Cdistance[id %d] = %f \n",k+1,Cdistance);
	    printf("Ddistance[id %d] = %f \n",k+2,Ddistance);
	    printf("Edistance[id %d] = %f \n",k+3,Edistance);
#endif
		/* this is made spu_sel by the compiler */
		// Aislan: Added the validation in case of the k % 4 > 0, for the invalid memory positions.
		// TODO: Improve it!!!

		if (__builtin_expect((min > distance), 0)) {
			min_id = k;
			min = distance;
		}
		if (__builtin_expect((min > Cdistance) && (k + 1 < number_of_centers), 0)) {
			min_id = k + 1;
			min = Cdistance;
		}
		if (__builtin_expect((min > Ddistance) && (k + 2 < number_of_centers), 0)) {
			min_id = k + 2;
			min = Ddistance;
		}
		if (__builtin_expect((min > Edistance) && (k + 3 < number_of_centers), 0)) {
			min_id = k + 3;
			min = Edistance;
		}

#ifdef DEBUG
	    printf("\n  *minDistance[id %d] = %f\n\n",min_id,min);
#endif

	    center1 += dimension * 4;
		center2 += dimension * 4;
		center3 += dimension * 4;
		center4 += dimension * 4;
	} //for each center mod 4

#ifdef DEBUG
	    printf("\nminDistance[id %d] = %f\n\n",min_id,min);
#endif

	return min_id;
}


/******************************************************************************
 *                                                                            *
 * Collect new centroid data from current centroids and the given chunk of    *
 * records.  The output data will be combined toghether from the other tasks  *
 * at the PPU.                                                                *
 *                                                                            *
 ******************************************************************************/
#pragma css task input(DIMENSIONS, CENTERS, number_of_records) \
                 input(records[number_of_records * DIMENSIONS], centers[CENTERS * DIMENSIONS]) \
                 inout(assigned_center[number_of_records]) \
                 inout(newcenters[CENTERS * DIMENSIONS], newcenters_histogram[CENTERS + 1])

void kmeans_calculate(int DIMENSIONS, int CENTERS, int number_of_records,
		float *records, float *centers, int *assigned_center,
		float *newcenters, int *newcenters_histogram) {

	int i, j;
	int min;

	for (i = 0; i < number_of_records; i++) {
		/* Find this record's nearest centroid */
		min = compare_to_centers(&records[i * DIMENSIONS], DIMENSIONS, CENTERS, centers);
		if (assigned_center[i] != min) {
			newcenters_histogram[CENTERS]++;
			assigned_center[i] = min;
		}
		/* Update centroid's cluster information */
		newcenters_histogram[min]++;
		for (j = 0; j < DIMENSIONS; j++)
			newcenters[min * DIMENSIONS + j] += records[i * DIMENSIONS + j];
	}
}



/*********************************            *********************************/
/*********************************  PPU CODE  *********************************/
/*********************************            *********************************/

/******************************************************************************
 *                                                                            *
 * Make a timer snapshot and, optionally, print the elapsed time since the    *
 * last time the function was called.                                         *
 *                                                                            *
 ******************************************************************************/
double time_int(int print) {

	double elapsed_seconds;
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


/******************************************************************************
 *                                                                            *
 * Combine results produced by the tasks and calculate new centroids for the  *
 * next clustering iteration.                                                 *
 *                                                                            *
 ******************************************************************************/
void recalculate_centers(int DIMENSIONS, int CENTERS, int num_spus,
		float *centers, float **newcenters, int **newcenters_histograms) {

	int i, j, k;

	/* Combine task results (co-ordinates and histograms) */
	for (k = 1; k < num_spus; k++)
		for (i = 0; i < CENTERS; i++) {
			newcenters_histograms[0][i] += newcenters_histograms[k][i];
			for (j = 0; j < DIMENSIONS; j++)
				newcenters[0][i * DIMENSIONS + j] += newcenters[k][i * DIMENSIONS + j];
		}
	/* Re-average centroids directly on the centroids array */
	for (i = 0; i < CENTERS; i++)
		for (j = 0; j < DIMENSIONS; j++) {
			if (newcenters_histograms[0][i] > 0)
				centers[i * DIMENSIONS + j] = newcenters[0][i * DIMENSIONS + j] / newcenters_histograms[0][i];
			else
				centers[i * DIMENSIONS + j] = -1;
		}
}

/******************************************************************************
 *                                                                            *
 * Main function.                                                             *
 *                                                                            *
 ******************************************************************************/
int main(int argc, char **argv) {
	/* Algorithm parameters */
	int NUMBER_OF_RECORDS, DIMENSIONS, CENTERS, CSS_NUM_SPUS;
	int number_of_records, block_size, block_records;
	int localstore_usage;
	/* Auxiliary variables */
	int k, i, j;
	int iteration, changed;
	/* Other computations */
	char *env_num_spus;
	int sizeofdistance;
	double gflops, elapsed_seconds;
	/* Data arrays */
	float *records;
	float *centers;
	int *assigned_centers;
	float **newcenters;
	int **newcenters_histograms;

	if (argc < 4) {
		printf("%s <data_points> <dim> <num_centers>\n", argv[0]);
		printf("eg: %s 200000 60 64\n", argv[0]);
		return 1;
	}

	NUMBER_OF_RECORDS = atoi(argv[1]);
	DIMENSIONS = atoi(argv[2]);
	CENTERS = atoi(argv[3]);

	/* Check number of centers (to avoid uchar overflow) */
	if (CENTERS > 256) {
		printf("Current program only supports 256 centers\n.");
		return 2;
	}

#ifdef CELL

	env_num_spus = getenv("CSS_NUM_SPUS");
	if (env_num_spus != NULL)
		CSS_NUM_SPUS = atoi(env_num_spus);
	else
		CSS_NUM_SPUS = 8;

#else

	CSS_NUM_SPUS = 1;

#endif

	block_size = DIMENSIONS * sizeof(float);
	block_records = 1;
	while (block_size < BLOCK_SIZE) {
		block_size *= 2;
		block_records *= 2;
	}

	localstore_usage = block_size + block_records * sizeof(int) + 2 * (CENTERS
			* DIMENSIONS * sizeof(float)) + (CENTERS + 1) * sizeof(int);

	/* Print precalculated information */
	printf("***********************************************\n");
	printf("***********************************************\n");
	printf("******                                   ******\n");
	printf("******          CellSs K-Means V2        ******\n");
	printf("******  BARCELONA SUPERCOMPUTING CENTER  ******\n");
	printf("******                                   ******\n");
	printf("******     Modified by:                  ******\n");
	printf("******         Aislan Gomide Foina       ******\n");
	printf("******                                   ******\n");
	printf("***********************************************\n");
	printf("***********************************************\n\n");
	printf("Number of data points is %d\n", NUMBER_OF_RECORDS);
	printf("Number of centers is %d\n", CENTERS);
	printf("Number of dimensions %d\n\n", DIMENSIONS);

	printf("Record array chunk size is %d\n", block_size);
	printf("Number of records per chunk is %d\n", block_records);
	printf("Predicted localstore usage is %d bytes\n\n", localstore_usage);

	/* Allocate space for data */
	records = memalign(128, NUMBER_OF_RECORDS * DIMENSIONS * sizeof(float));
	centers = memalign(128, CENTERS * DIMENSIONS * sizeof(float));
	assigned_centers = memalign(128, NUMBER_OF_RECORDS * sizeof(int));
	newcenters = malloc(CSS_NUM_SPUS * sizeof(float *));
	newcenters_histograms = malloc(CSS_NUM_SPUS * sizeof(int *));

	for (k = 0; k < CSS_NUM_SPUS; k++) {
		newcenters[k] = memalign(128, CENTERS * DIMENSIONS * sizeof(float));
		newcenters_histograms[k] = memalign(128, (CENTERS + 1) * sizeof(int));
	}

	time_int(0);

	/* Make record data */
	for (i = 0; i < NUMBER_OF_RECORDS; i++)
		for (j = 0; j < DIMENSIONS; j++)
#ifdef DEBUG
			records[i * DIMENSIONS + j] = rand() % 20;
#else
			records[i * DIMENSIONS + j] = rand() / 100000.0f + j;
#endif

	/* Initialize the centers */
	for (i = 0; i < CENTERS; i++)
		memcpy(&centers[i * DIMENSIONS], &records[i * DIMENSIONS], DIMENSIONS * sizeof(float));

	/* Clear assigned centers */
	memset(assigned_centers, -1, NUMBER_OF_RECORDS * sizeof(int));

	time_int(1);

	changed = 0;
	iteration = 0;
	gflops = 0.0;
	sizeofdistance = DIMENSIONS * 3 + 1;

#pragma css start

	time_int(0);

	/* Main loop */
	do {

		for (k = 0; k < CSS_NUM_SPUS; k++) {
			memset(newcenters[k], 0, CENTERS * DIMENSIONS * sizeof(float));
			memset(newcenters_histograms[k], 0, (CENTERS + 1) * sizeof(int));
		}

		k = 0;
		for (i = 0; i < NUMBER_OF_RECORDS; i += block_records) {
			number_of_records = (i + block_records > NUMBER_OF_RECORDS ? NUMBER_OF_RECORDS - i : block_records);

			kmeans_calculate(DIMENSIONS, CENTERS, number_of_records, &records[i
					* DIMENSIONS], centers, &assigned_centers[i],
					newcenters[k], newcenters_histograms[k]);

			k = (k + 1) % CSS_NUM_SPUS;
		}

		/* Delay accounting calculations in order to launch tasks as
		 * soon as possible */
		if (changed)
			printf("TOTAL CHANGED IS %d\n\n", changed);

		gflops += (double) NUMBER_OF_RECORDS * CENTERS * sizeofdistance;

		iteration++;
		printf("EXECUTING ITERATION #%d\n", iteration);

#ifdef PAUSE
		scanf("%d");
#endif

#pragma css barrier

		changed = 0;
		for (k = 0; k < CSS_NUM_SPUS; k++)
			changed += newcenters_histograms[k][CENTERS];

		if ((float) changed / NUMBER_OF_RECORDS < TERMINATION_THRESHOLD) {
			printf("TOTAL CHANGED IS %d\n\n", changed);
			printf("Number of changed records has fallen below the threshold of %f\n\n", TERMINATION_THRESHOLD);
			break;
		}
		if (iteration >= MAX_ITERATIONS) {
			printf("TOTAL CHANGED IS %d\n\n", changed);
			printf("We are at the maximum number of iterations ...\n\n");
			break;
		}

		recalculate_centers(DIMENSIONS, CENTERS, CSS_NUM_SPUS, centers,
				newcenters, newcenters_histograms);

	} while (1);

    elapsed_seconds = time_int(1);

#pragma css finish

#ifdef REPORT

    printf("\n######### REPORT #########\n\n");

//    if((DIMENSIONS < 10) && (NUMBER_OF_RECORDS < 100)) {
		for (i = 0;  i < NUMBER_OF_RECORDS;  i++) {
			pVector("PNTf",i,&records[i*DIMENSIONS],DIMENSIONS);
			printf(" CNTid = %d\n",assigned_centers[i]);
		}
		printf("\n\n");

		for (i = 0;  i < CENTERS;  i++) {
			pVector("CNTf",i,&centers[i*DIMENSIONS],DIMENSIONS);
			printf("\n");
		}
//    }

#endif

	printf("\nNumber of ITERATIONS = #%d\n", iteration);


	gflops = gflops / (1024.0 * 1024.0 * 1024.0);
	printf(" ROUGH ESTIMATED GFLOPS=%lf\n\n\n", gflops / elapsed_seconds);
	return 0;
}

