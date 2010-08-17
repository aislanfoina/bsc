/*
 * ProfileAnalyses.c
 *
 *  Created on: Aug 17, 2010
 *      Author: aislan
 */

#include "ProfileAnalyses.h"

int getProfile(profile_t *profile, char *type, char *movieDb, MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	int i;

	char query[4098];
	sprintf(query, "select * from %s_%s where customer_id = %d", type, movieDb, profile->id);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

//	int numrows = mysql_num_rows(res);

	if ((row = mysql_fetch_row(res)) != NULL) {

		for(i = 0; i < NUMBER_OF_GEN; i++) {
			profile->genres[i] = (float) atof(row[i+1]);
		}
	}
	else {
		for(i = 0; i < NUMBER_OF_GEN; i++) {
			profile->genres[i] = 0;
		}
		return -1;
	}

	return 0;
}

int subProf(profile_t *dst, profile_t *sub1, profile_t *sub2) {
	int i;

	for(i = 0; i < NUMBER_OF_GEN; i++) {
		dst->genres[i] = sub1->genres[i]-sub2->genres[i];
	}

	return 0;
}
