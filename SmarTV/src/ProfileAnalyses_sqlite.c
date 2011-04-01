/*
 * ProfileAnalyses.c
 *
 *  Created on: Aug 17, 2010
 *      Author: aislan
 */

#include "ProfileAnalyses_sqlite.h"

int getProfile(profile_t *profile, int chunk_size, char *type, char *movieDb, char *sqlite3_db) {
	sqlite3 *db;
	sqlite3_stmt *pStmt;

	if(sqlite3_open_v2(sqlite3_db, &db, SQLITE_OPEN_FULLMUTEX, NULL)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
	}

	int i;

	char query[4098];
	char array_ids[chunk_size*10];

	sprintf(array_ids, "%d", profile[0].id);
	for(i = 1; i < chunk_size; i++)
		sprintf(array_ids, "%s, %d", array_ids, profile[i].id);

	sprintf(query, "select customer_id, cluster from profiles_Data_%s where customer_id in (%s)", movieDb, array_ids);

	sqlite3_prepare_v2(db, query, -1, &pStmt, 0);
	int numrows = 0;
	int run;
	if(sqlite3_step(pStmt)==SQLITE_ROW)
		run = 1;
	else
		run = 0;

	while(run && (numrows<chunk_size)) {
		if(profile[numrows].id == sqlite3_column_int(pStmt, 0)) {
			profile[numrows].cluster = sqlite3_column_int(pStmt, 1);
			if(!sqlite3_step(pStmt)==SQLITE_ROW)
				run = 0;
		}
		else
			profile[numrows].cluster = 0;
//		if(numrows+1 >= chunk_size)
//			run = 0;
		numrows++;
	}

	sqlite3_finalize(pStmt);
	sqlite3_close(db);
/*
	if(!numrows) {
		profile[0]->cluster = 0;
		return -1;
	}
*/
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

	mysql_free_result(res);

	return 0;
}
