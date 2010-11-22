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

	sprintf(query, "select cluster from profiles_Data_%s where customer_id = %d", movieDb, profile->id);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	if ((row = mysql_fetch_row(res)) != NULL) {
		profile->cluster = atoi(row[0]);
	}
	else {
		profile->cluster = 0;
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

int getFriendsProfile(profile_t *profile, profile_t *ret_profiles, char *type, char *movieDb, MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	int i;

	char query[4098];
	sprintf(query, "select customer_id from %s_%s where cluster = %d", type, movieDb, profile->cluster);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	ret_profiles = malloc(numrows*sizeof(profile_t));

	i = 0;

	if ((row = mysql_fetch_row(res)) != NULL) {
		ret_profiles[i].id = atoi(row[0]);
		ret_profiles[i].cluster = profile->cluster;

		i++;
	}

	return 0;
}
