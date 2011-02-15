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


int main(void) {
	int run = 1;
	profile_t *pList;
	int *ids;
	int pListChange;

	int i;

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char *server = "localhost";
	char *user = "root";
	char *password = "root"; /* set me first */
	char *database = "sandbox";

	char query[4098];

	conn = mysql_init(NULL);

	/* Connect to database */
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

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
		int numIds = getIds(ids, conn);

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
			for (i = 0; i < pListLen; i++) {

				if((i%1000) == 0 || i == pListLen-1)
					printf("%d of %d (%f%%) profiles loaded...\n", i, pListLen, ((float)i/(float)(pListLen-1))*100);

				getProfile(&pList[i], "profiles_RateCntPer", "Allmovie", conn);
			}
			// colaborative recommendations

			float *rate = malloc(sizeof(float));

//			int movieId = 1;
			int *movieIds;

			movieIds = malloc(100000*sizeof(int));

			for (i = 0; i < pListLen; i++) {

				if((i%10) == 0 || i == pListLen-1)
					printf("%d of %d (%f%%) predictions processed...\n", i, pListLen, ((float)i/(float)(pListLen-1))*100);

				int numMoviesIds = getMovieIds(&pList[i], movieIds, conn);

				int j;

				for (j = 0; j < numMoviesIds; j++) {
/*					float *probeRate = malloc(sizeof(float));
					fixProbe(&pList[i], movieIds[j], probeRate, "ratings", conn);

					sprintf(query, "update probe set rating = %f where movie_id = %d and customer_id = %d;", *probeRate, movieIds[j], pList[i].id);

					if (mysql_query(conn, query)) {
						fprintf(stderr, "%s\n", mysql_error(conn));
						exit(1);
					}
					free(probeRate);*/

					getRate(&pList[i], movieIds[j], rate, "ratings", "Allmovie", conn);
//					printf("\tRate for movie %d and user %d[%d] is %f\n", movieIds[j], pList[i].id, pList[i].cluster, *rate);

					sprintf(query, "update probe set prediction = %f where movie_id = %d and customer_id = %d;", *rate, movieIds[j], pList[i].id);

					if (mysql_query(conn, query)) {
						fprintf(stderr, "%s\n", mysql_error(conn));
						exit(1);
					}
				}
			}
			printf("\n\n");


			// content based recommendations

			// compare with the medium profile to get the like, dontcare, notlike for each profile
/*
			profile_t meanProf;
			meanProf.id = 0;
			getProfile(&meanProf, "profiles_RateCntPer", "Allmovie", conn);

			for (i = 0; i < pListLen; i++) {
				subProf(&pList[i], &pList[i], &meanProf);
			}
*/
			// combine recommendations

				// get biggest number of likes and smallest number of notlike (create a score for each genre)
				// look at the options available and choose the ones with this genre

			// send the recommendations for each one, and for the group
//			run = 0;
			free(rate);
			free(movieIds);
		}

		free(ids);
		free(pList);
		time_int(1);


	}
	maintime_int(1);
	mysql_close(conn);

	return EXIT_SUCCESS;
}
