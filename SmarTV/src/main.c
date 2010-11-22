/*
 * main.c
 *
 *  Created on: Aug 6, 2010
 *      Author: aislan
 */

#include "SSACT.h"

int main(void) {
	int run = 1;
	profile_t *pList;
	int pListChange;

	int i;

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	char *server = "localhost";
	char *user = "root";
	char *password = "root"; /* set me first */
	char *database = "sandbox";

	conn = mysql_init(NULL);

	/* Connect to database */
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

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
		int ids[] = { 123120, 2118461, 1932594, 2143500, 1977959, 1570292,
				2482738, 676682, 307530, 1228542, 1404976, 2311335, 780341 };

		int pListLen = sizeof(ids)/sizeof(int);
		pList = malloc(pListLen * sizeof(profile_t));
		for(i = 0; i < pListLen; i++) {
			pList[i].id = ids[i];
		}

		pListChange = 1;

		if(pListChange) {
			int i;
			// get all profiles
			for (i = 0; i < pListLen; i++) {
				getProfile(&pList[i], "profiles_RateCntPer", "Allmovie", conn);
			}
			// colaborative recommendations

			float *rate = malloc(sizeof(float));

			int movieId = 1;

			for (i = 0; i < pListLen; i++) {
				getRate(&pList[i], movieId, rate, "Allmovie", conn);
				printf(" Rate for movie %d and user %d[%d] is %f\n", movieId, pList[i].id, pList[i].cluster, *rate);
			}
			printf("\n\n");


			// content based recommendations

			// compare with the medium profile to get the like, dontcare, notlike for each profile
			profile_t meanProf;
			meanProf.id = 0;
			getProfile(&meanProf, "profiles_RateCntPer", "Allmovie", conn);

			for (i = 0; i < pListLen; i++) {
				subProf(&pList[i], &pList[i], &meanProf);
			}
			// combine recommendations
				// get biggest number of likes and smallest number of notlike (create a score for each genre)
				// look at the options available and choose the ones with this genre

			// send the recommendations for each one, and for the group

		}

	}

	mysql_close(conn);

	return EXIT_SUCCESS;
}
