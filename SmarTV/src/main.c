/*
 * main.c
 *
 *  Created on: Aug 6, 2010
 *      Author: aislan
 */

#include "SSACT.h"

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

double time_int(int print) {

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

double rate_speed(int print, int rates) {

        double elapsed_seconds = 0;
        static struct timeval t1; /* var for previous time stamp */
        static struct timeval t2; /* var of current time stamp */

        if (gettimeofday(&t2, NULL) == -1) {
                perror("gettimeofday");
                exit(9);
        }

        if (print) {
                elapsed_seconds = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) * 1e-6;
                printf("Rates per seconds = %.2f r/s \n", rates/elapsed_seconds);
        }

        t1 = t2;
        return elapsed_seconds;
}

int main(void) {
	int run = 1;
	int collaborative = 1;
	int content = 0;
	int merge = 0;

	profile_t *pList;
	int *ids;
	int pListChange;

	int i;
	int total_rate_cnt = 0;

#ifdef USE_MYSQL

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char *server = "localhost";
	char *user = "root";
	char *password = "root"; // set me first
	char *database = "sandbox";

	int chunk_size = 1;

	char query[4098];

	conn = mysql_init(NULL);

	/* Connect to database */
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

#else

	char *sqlite3_db = "sandbox.db";
	int chunk_size = 100;

#endif

	printf("Start timer!\n");
	maintime_int(0);
	time_int(0);

	while (run) {

		/**
		 *
		 * Get id from bluetooth every refresh rate (10 sec)
		 *
		 */

		/**
		 *
		 * Compare with the actual presence list. If changes, update presence list.
		 *
		 */

//		int ids[] = { 123120, 2118461, 1932594, 2143500, 1977959 };
//		int ids[] = { 123120, 2118461, 1932594, 2143500, 1977959, 1570292,
//				2482738, 676682, 307530, 1228542, 1404976, 2311335, 780341 };

		ids = malloc(400000*sizeof(int));

#ifdef USE_MYSQL

		int numIds = getIds(ids, conn);

#else
		int numIds = getIds(ids, sqlite3_db);

#endif

		int pListLen = numIds;
		pList = malloc(pListLen * sizeof(profile_t));
		for(i = 0; i < pListLen; i++) {
			pList[i].id = ids[i];
		}

		pListChange = 1;

		if(numIds == 0) {
			run = 0;
			pListChange = 0;
		}

		if(pListChange) {
			int i;
			// get all profiles
			for (i = 0; i < pListLen; i+=chunk_size) {

				if((i%10) == 0 || i == pListLen-1)
					printf("%d of %d (%f%%) profiles loaded...\n", i, pListLen, ((float)i/(float)(pListLen-1))*100);
#ifdef USE_MYSQL
				getProfile(&pList[i], "profiles_RateCntPer", "Allmovie", conn);
#else
				getProfile(&pList[i], chunk_size, "profiles_RateCntPer", "Allmovie", sqlite3_db);
#endif
			}

			float *rate = malloc(sizeof(float));
			int rate_cnt = 0;
			rate_speed(0, rate_cnt);

//			int movieId = 1;
			int *movieIds;

			movieIds = malloc(100000*sizeof(int));
			for (i = 0; i < pListLen; i++) {

				if((i%10) == 0 || i == pListLen-1) {
					printf("%d of %d (%f%%) profiles processed...\n", i, pListLen, ((float)i/(float)(pListLen-1))*100);
					rate_speed(1, rate_cnt);
					total_rate_cnt += rate_cnt;
					rate_cnt = 0;
				}
				// Collaborative recommendation

				if(collaborative) {

#ifdef USE_MYSQL
					int numMoviesIds = getMovieIds(&pList[i], movieIds, conn);
#else
					int numMoviesIds = getMovieIds(&pList[i], movieIds, sqlite3_db);
#endif
					int j;

					for (j = 0; j < numMoviesIds; j++) {
	//					float *probeRate = malloc(sizeof(float));
	//					fixProbe(&pList[i], movieIds[j], probeRate, "ratings", conn);

	//					sprintf(query, "update probe set rating = %f where movie_id = %d and customer_id = %d;", *probeRate, movieIds[j], pList[i].id);

	//					if (mysql_query(conn, query)) {
	//						fprintf(stderr, "%s\n", mysql_error(conn));
	//						exit(1);
	//					}
	//					free(probeRate);

#ifdef USE_MYSQL
						getRate(&pList[i], movieIds[j], rate, "ratings_bellkor", "Allmovie", conn);
						rate_cnt++;
	//					printf("\tRate for movie %d and user %d[%d] is %f\n", movieIds[j], pList[i].id, pList[i].cluster, *rate);
/*
						sprintf(query, "update probe set prediction = %f where movie_id = %d and customer_id = %d;", *rate, movieIds[j], pList[i].id);

						if (mysql_query(conn, query)) {
							fprintf(stderr, "%s\n", mysql_error(conn));
							exit(1);
						}*/
#else
						getRate(&pList[i], movieIds[j], rate, "ratings_bellkor", "Allmovie", sqlite3_db);
						rate_cnt++;
#endif
					}
				}
				if(content) {

					// content based recommendations

					// compare with the medium profile to get the like, dontcare, notlike for each profile

					profile_t meanProf;
					meanProf.id = 0;
#ifdef USE_MYSQL
					getProfile(&meanProf, "profiles_RateCntPer", "Allmovie", conn);
#else
#endif

					for (i = 0; i < pListLen; i++) {
						subProf(&pList[i], &pList[i], &meanProf);
					}
				}
				if(merge) {
					// combine recommendations

						// get biggest number of likes and smallest number of notlike (create a score for each genre)
						// look at the options available and choose the ones with this genre

					// send the recommendations for each one, and for the group
				}

			}
			printf("\n\n");
			run = 0;
			free(rate);
			free(movieIds);
		}

		free(ids);
		free(pList);
		time_int(1);
		printf("Total rates: %d\n\n", total_rate_cnt);

	}
	maintime_int(1);
#ifdef USE_MYSQL
	mysql_close(conn);
#else
#endif

	return EXIT_SUCCESS;
}
