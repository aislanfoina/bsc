#include <spu_intrinsics.h>
#include <sys/time.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * When the ratio of record movement across custers (led by centroids) hits
 * this low bound, the algorithms does not perform any more iterations.
 */
#define TERMINATION_THRESHOLD  0.01f

/*
 * Iteration hard limit.
 */
#define MAX_ITERATIONS         100


/*
 * Size hint for the record block.
 */
#define BLOCK_SIZE             8192



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
int compare_to_centers (float  *record,
                        int     record_length,
                        int     number_of_centers,
                        float  *centers)
{
  short k;
  short i,j;
  short min_id=0;
  float min = 1e20;
  short blocksize=12;
  short loopcount = record_length / blocksize;
  short extra = record_length % blocksize;
  float *data2=centers;//+k*(cb.record_length);
  float *data1=record;

  float distance;
  float distance_3 = 0.0;
  float distance_4 = 0.0;
  float distance_5= 0.0;
  int offsetX = record_length;
  float *data3=centers+offsetX;//+k*(record_length);
  float Cdistance;
  float Cdistance_3 = 0.0;
  float Cdistance_4 = 0.0;
  float Cdistance_5= 0.0;

  offsetX +=record_length;
  float *data4=centers+offsetX;//+k*(cb.record_length);
  float Ddistance;
  float Ddistance_3 = 0.0;
  float Ddistance_4 = 0.0;
  float Ddistance_5= 0.0;

  offsetX +=record_length;
  float *data5=centers+offsetX;//+k*(cb.record_length);
  float Edistance;
  float Edistance_3 = 0.0;
  float Edistance_4 = 0.0;
  float Edistance_5= 0.0;

  for(k=0; k< number_of_centers-4; k+=4) //for each center
    {
      distance_3 = 0.0;
      distance_4 = 0.0;
      distance_5= 0.0;
      vector float* varray1 = (vector float*)(data1);
      vector float* varray2 = (vector float*)(data2);
      vector float result;
      vector float result2;
      vector float result3;

      Cdistance_3 = 0.0;
      Cdistance_4 = 0.0;
      Cdistance_5= 0.0;
      vector float* Cvarray2 = (vector float*)(data3);
      vector float Cresult;
      vector float Cresult2;
      vector float Cresult3;

      Ddistance_3 = 0.0;
      Ddistance_4 = 0.0;
      Ddistance_5= 0.0;
      vector float* Dvarray2 = (vector float*)(data4);
      vector float Dresult;
      vector float Dresult2;
      vector float Dresult3;


      Edistance_3 = 0.0;
      Edistance_4 = 0.0;
      Edistance_5= 0.0;
      vector float* Evarray2 = (vector float*)(data5);
      vector float Eresult;
      vector float Eresult2;
      vector float Eresult3;
      vector float temp,Ctemp,Dtemp,Etemp;
      int index=0;

      //accumulators
      vector float C1acc = {0,0,0,0};
      vector float C2acc = {0,0,0,0};
      vector float C3acc = {0,0,0,0};
      vector float C4acc = {0,0,0,0};
      for(i=0;i<loopcount;i++)
        {
          //prof_cp1();
          result = spu_sub(varray1[index],varray2[index]);
          result2 = spu_sub(varray1[index+1],varray2[index+1]);
          result3 = spu_sub(varray1[index+2],varray2[index+2]);
          Cresult = spu_sub(varray1[index],Cvarray2[index]);
          Cresult2 = spu_sub(varray1[index+1],Cvarray2[index+1]);
          Cresult3 = spu_sub(varray1[index+2],Cvarray2[index+2]);
          Dresult = spu_sub(varray1[index],Dvarray2[index]);
          Dresult2 = spu_sub(varray1[index+1],Dvarray2[index+1]);
          Dresult3 = spu_sub(varray1[index+2],Dvarray2[index+2]);
          Eresult = spu_sub(varray1[index],Evarray2[index]);
          Eresult2 = spu_sub(varray1[index+1],Evarray2[index+1]);
          Eresult3 = spu_sub(varray1[index+2],Evarray2[index+2]);

          result = spu_mul(result,result);
          result2 = spu_mul(result2,result2);
          result3 = spu_mul(result3,result3);
          Cresult = spu_mul(Cresult,Cresult);
          Cresult2 = spu_mul(Cresult2,Cresult2);
          Cresult3 = spu_mul(Cresult3,Cresult3);
          Dresult = spu_mul(Dresult,Dresult);
          Dresult2 = spu_mul(Dresult2,Dresult2);
          Dresult3 = spu_mul(Dresult3,Dresult3);
          Eresult = spu_mul(Eresult,Eresult);
          Eresult2 = spu_mul(Eresult2,Eresult2);
          Eresult3 = spu_mul(Eresult3,Eresult3);
          // these can be SIMD'd
          //just keep in vector accumulators until the looping ends

          /*
          C1acc = spu_madd(result,result,C1acc);
          C2acc = spu_madd(Cresult,Cresult,C2acc);
          C3acc = spu_madd(Dresult,Dresult,C3acc);
          C4acc = spu_madd(Eresult,Eresult,C4acc);
          //printf("mul1 is %f\n",spu_extract(C1acc,0)+spu_extract(C1acc,1)+spu_extract(C1acc,2)+spu_extract(C1acc,3));

          C1acc = spu_madd(result2,result2,C1acc);
          C2acc = spu_madd(Cresult2,Cresult2,C2acc);
          C3acc = spu_madd(Dresult2,Dresult2,C3acc);
          C4acc = spu_madd(Eresult2,Eresult2,C4acc);

          C1acc = spu_madd(result3,result3,C1acc);
          C2acc = spu_madd(Cresult3,Cresult3,C2acc);
          C3acc = spu_madd(Dresult3,Dresult3,C3acc);
          C4acc = spu_madd(Eresult3,Eresult3,C4acc);
          */


          C1acc = spu_add(result,result2);
          C2acc = spu_add(Cresult,Cresult2);
          C3acc = spu_add(Dresult,Dresult2);
          C4acc = spu_add(Eresult,Eresult2);
          C1acc = spu_add(C1acc,result3);
          C2acc = spu_add(C2acc,Cresult3);
          C3acc = spu_add(C3acc,Dresult3);
          C4acc = spu_add(C4acc,Eresult3);

          //distance=spu_extract(C1acc,0)+spu_extract(C1acc,1)+spu_extract(C1acc,2)+spu_extract(C1acc,3);
          //printf("val is %f\n",distance);
          //getchar();
          index = index+3;
          //prof_cp2();
        }
      distance=spu_extract(C1acc,0)+spu_extract(C1acc,1)+spu_extract(C1acc,2)+spu_extract(C1acc,3);
      Cdistance=spu_extract(C2acc,0)+spu_extract(C2acc,1)+spu_extract(C2acc,2)+spu_extract(C2acc,3);
      Ddistance=spu_extract(C3acc,0)+spu_extract(C3acc,1)+spu_extract(C3acc,2)+spu_extract(C3acc,3);
      Edistance=spu_extract(C4acc,0)+spu_extract(C4acc,1)+spu_extract(C4acc,2)+spu_extract(C4acc,3);


      if(__builtin_expect((extra>0),0))
        {
          for(i=0;i<extra;i++)
            {
              float distance1 =data1[index+i]-data2[index+i];
              distance += distance1*distance1;
              float Cdistance1 =data1[index+i]-data3[index+i];
              Cdistance += Cdistance1*Cdistance1;
              float Ddistance1 =data1[index+i]-data4[index+i];
              Ddistance += Ddistance1*Ddistance1;
              float Edistance1 =data1[index+i]-data5[index+i];
              Edistance += Edistance1*Edistance1;
            }
        }
      //vector float min1 =spu_promote(min,0);
      //vector float dis1 =spu_promote(distance,0);
      //vector unsigned int sel = spu_cmpgt(min1,dis1);
      //min = spu_extract(spu_sel(dis1,min1,sel),0);
      //min_id = spu_extract(spu_sel(k,min_id,sel),0);
      //min = spu_extract(select);

      /* this is made spu_sel by the compiler */
      if(__builtin_expect((min > distance),0))
        {
          min_id=k;
          min=distance;
        }
      if(__builtin_expect((min > Cdistance),0))
        {
          min_id=k+1;
          min=Cdistance;
        }
      if(__builtin_expect((min > Ddistance),0))
        {
          min_id=k+2;
          min=Ddistance;
        }
      if(__builtin_expect((min > Edistance),0))
        {
          min_id=k+3;
          min=Edistance;
        }
      data2+=record_length*4;
      data3+=record_length*4;
      data4+=record_length*4;
      data5+=record_length*4;
    } //for each center mod 4
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
void kmeans_calculate (int     DIMENSIONS,
                       int     CENTERS,
                       int     number_of_records,
                       float  *records,
                       float  *centers,
                       int    *assigned_center,
                       float  *newcenters,
                       int    *newcenters_histogram)
{
        int  i, j;
        int  min;

        for (i = 0;  i < number_of_records;  i++)
        {
                /* Find this record's nearest centroid */
                min     = compare_to_centers(&records[i * DIMENSIONS],
                                             DIMENSIONS, CENTERS, centers);
                if (assigned_center[i] != min)
                {
                        newcenters_histogram[CENTERS]++;
                        assigned_center[i] = min;
                }
                /* Update centroid's cluster information */
                newcenters_histogram[min]++;
                for (j = 0;  j < DIMENSIONS;  j++)
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
double time_int (int print)
{

        double                 elapsed_seconds;
        static struct timeval  t1;  /* var for previous time stamp */
        static struct timeval  t2;  /* var of current time stamp */

        if (gettimeofday(&t2, NULL) == -1)
        {
                perror("gettimeofday");
                exit(9);
        }

        if (print)
        {
                elapsed_seconds = (t2.tv_sec - t1.tv_sec) +
                                  (t2.tv_usec - t1.tv_usec) * 1e-6;
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
void recalculate_centers (int      DIMENSIONS,
                          int      CENTERS,
                          int      num_spus,
                          float   *centers,
                          float  **newcenters,
                          int    **newcenters_histograms)
{
        int  i, j, k;

        /* Combine task results (co-ordinates and histograms) */
        for (k = 1;  k < num_spus;  k++)
                for (i = 0;  i < CENTERS;  i++)
                {
                        newcenters_histograms[0][i] += newcenters_histograms[k][i];
                        for (j = 0;  j < DIMENSIONS;  j++)
                                newcenters[0][i*DIMENSIONS+j] += newcenters[k][i*DIMENSIONS+j];
                }
        /* Re-average centroids directly on the centroids array */
        for (i = 0;  i < CENTERS;  i++)
                for (j = 0;  j < DIMENSIONS;  j++)
                {
                        if (newcenters_histograms[0][i] > 0)
                                centers[i*DIMENSIONS+j] = newcenters[0][i*DIMENSIONS+j] / newcenters_histograms[0][i];
                        else
                                centers[i*DIMENSIONS+j] = -1;
                }
}


/******************************************************************************
 *                                                                            *
 * Main function.                                                             *
 *                                                                            *
 ******************************************************************************/
int main (int argc, char **argv)
{
        /* Algorithm parameters */
        int      NUMBER_OF_RECORDS, DIMENSIONS, CENTERS, CSS_NUM_SPUS;
        int      number_of_records, block_size, block_records;
        int      localstore_usage;
        /* Auxiliary variables */
        int      k, i, j;
        int      iteration, changed;
        /* Other computations */
        char    *env_num_spus;
        int      sizeofdistance;
        double   gflops, elapsed_seconds;
        /* Data arrays */
        float   *records;
        float   *centers;
        int     *assigned_centers;
        float  **newcenters;
        int    **newcenters_histograms;

        if (argc < 4)
        {
                printf("%s <data_points> <dim> <num_centers>\n", argv[0]);
                printf("eg: %s 200000 60 64\n", argv[0]);
                return 1;
        }

        NUMBER_OF_RECORDS  = atoi(argv[1]);
        DIMENSIONS         = atoi(argv[2]);
        CENTERS            = atoi(argv[3]);

        /* Check number of centers (to avoid uchar overflow) */
        if (CENTERS > 256)
        {
                printf("Current program only supports 256 centers\n.");
                return 2;
        }

        env_num_spus = getenv("CSS_NUM_SPUS");
        if (env_num_spus != NULL)
                CSS_NUM_SPUS = atoi(env_num_spus);
        else
                CSS_NUM_SPUS = 8;

        block_size    = DIMENSIONS * sizeof(float);
        block_records = 1;
        while (block_size < BLOCK_SIZE)
        {
                block_size    *= 2;
                block_records *= 2;
        }

        localstore_usage = block_size + block_records * sizeof(int) +
                           2 * (CENTERS * DIMENSIONS * sizeof(float)) +
                           (CENTERS + 1) * sizeof(int);

        /* Print precalculated information */
        printf("***********************************************\n");
        printf("***********************************************\n");
        printf("******                                   ******\n");
        printf("******          CellSs K-Means           ******\n");
        printf("******  BARCELONA SUPERCOMPUTING CENTER  ******\n");
        printf("******                                   ******\n");
        printf("******     ORIGINAL CBE VERSION FROM     ******\n");
        printf("******       OHIO STATE UNIVERSITY       ******\n");
        printf("******                                   ******\n");
        printf("***********************************************\n");
        printf("***********************************************\n\n");
        printf("Number of data points is %d\n",     NUMBER_OF_RECORDS);
        printf("Number of centers is %d\n",         CENTERS);
        printf("Number of dimensions %d\n\n",       DIMENSIONS);

        printf("Record array chunk size is %d\n",     block_size);
        printf("Number of records per chunk is %d\n", block_records);
        printf("Predicted localstore usage is %d bytes\n\n", localstore_usage);

        /* Allocate space for data */
        records               = memalign(128, NUMBER_OF_RECORDS * DIMENSIONS * sizeof(float));
        centers               = memalign(128, CENTERS * DIMENSIONS * sizeof(float));
        assigned_centers      = memalign(128, NUMBER_OF_RECORDS * sizeof(int));
        newcenters            = malloc(CSS_NUM_SPUS * sizeof(float *));
        newcenters_histograms = malloc(CSS_NUM_SPUS * sizeof(int *));
        for (k = 0;  k < CSS_NUM_SPUS;  k++)
        {
                newcenters[k]            = memalign(128, CENTERS * DIMENSIONS * sizeof(float));
                newcenters_histograms[k] = memalign(128, (CENTERS + 1) * sizeof(int));
        }

        time_int(0);

        /* Make record data */
        for (i = 0;  i < NUMBER_OF_RECORDS;  i++)
                for (j = 0;  j < DIMENSIONS;  j++)
                        records[i * DIMENSIONS + j] = rand() / 100000.0f + j;

        /* Initialize the centers */
        for (i = 0;  i < CENTERS;  i++)
                memcpy(&centers[i * DIMENSIONS], &records[i * DIMENSIONS], DIMENSIONS * sizeof(float));

        /* Clear assigned centers */
        memset(assigned_centers, -1, NUMBER_OF_RECORDS * sizeof(int));

        time_int(1);

        changed        = 0;
        iteration      = 0;
        gflops         = 0.0;
        sizeofdistance = DIMENSIONS * 3 + 1;

        #pragma css start
        time_int(0);

        /* Main loop */
        do {

                for (k = 0;  k < CSS_NUM_SPUS;  k++)
                {
                        memset(newcenters[k], 0, CENTERS * DIMENSIONS * sizeof(float));
                        memset(newcenters_histograms[k], 0, (CENTERS + 1) * sizeof(int));
                }

                k = 0;
                for (i = 0;  i < NUMBER_OF_RECORDS;  i += block_records)
                {
                        number_of_records = (i + block_records > NUMBER_OF_RECORDS ? NUMBER_OF_RECORDS - i : block_records);

                        kmeans_calculate(DIMENSIONS, CENTERS, number_of_records,
                                         &records[i * DIMENSIONS], centers,
                                         &assigned_centers[i], newcenters[k],
                                         newcenters_histograms[k]);

                        k = (k + 1) % CSS_NUM_SPUS;
                }

                /* Delay accounting calculations in order to launch tasks as
                 * soon as possible */
                if (changed)
                        printf("TOTAL CHANGED IS %d\n\n", changed);
                gflops += (double) NUMBER_OF_RECORDS * CENTERS * sizeofdistance;
                iteration++;
                printf("EXECUTING ITERATION #%d\n", iteration);

                #pragma css barrier
                changed = 0;
                for (k = 0;  k < CSS_NUM_SPUS;  k++)
                        changed += newcenters_histograms[k][CENTERS];

                if ((float) changed / NUMBER_OF_RECORDS < TERMINATION_THRESHOLD)
                {
                        printf("TOTAL CHANGED IS %d\n\n", changed);
                        printf("Number of changed records has fallen below the threshold of %f\n\n", TERMINATION_THRESHOLD);
                        break;
                }
                if (iteration >= MAX_ITERATIONS)
                {
                        printf("TOTAL CHANGED IS %d\n\n", changed);
                        printf("We are at the maximum number of iterations ...\n\n");
                        break;
                }

                recalculate_centers(DIMENSIONS, CENTERS, CSS_NUM_SPUS,
                                    centers, newcenters, newcenters_histograms);
        } while (1);
        elapsed_seconds = time_int(1);
        #pragma css finish

        gflops = gflops / (1024.0 * 1024.0 * 1024.0);
        printf(" ROUGH ESTIMATED GFLOPS=%lf\n\n\n", gflops / elapsed_seconds);
        return 0;
}

