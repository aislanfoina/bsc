/*
 * ColaborativeRecommender.c
 *
 *  Created on: Nov 22, 2010
 *      Author: aislan
 */

#include "ColaborativeRecommender_sqlite.h"

int getRate(profile_t *profile, int idMovie, float *rate, char *rateDb, char *movieDb, char *sqlite3_db) {
	sqlite3 *db;
	sqlite3_stmt *pStmt;

	if(sqlite3_open_v2(sqlite3_db, &db, SQLITE_OPEN_FULLMUTEX, NULL)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
	}

	char query[4098];
	sprintf(query, "select sum(rating) / count(rating) as meanRate, count(rating) as countRate from %s where movie_id = %d "
			"and status = 1 and customer_id in "
			"(select customer_id from profiles_Data_%s where cluster = %d);",
			rateDb, idMovie, movieDb, profile->cluster);

	sqlite3_prepare_v2(db, query, -1, &pStmt, 0);
	if(sqlite3_step(pStmt)==SQLITE_ROW) {
		if(sqlite3_column_int(pStmt, 0) > 0)
			*rate = sqlite3_column_int(pStmt, 0);
		else
			*rate = -2;
	}
	else
		*rate = -3;

	sqlite3_finalize(pStmt);
	sqlite3_close(db);

	return 0;
}
