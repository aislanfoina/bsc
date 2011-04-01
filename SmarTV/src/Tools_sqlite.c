/*
 * ColaborativeRecommender.c
 *
 *  Created on: Nov 22, 2010
 *      Author: aislan
 */

#include "Tools_sqlite.h"

int cleanRatings(char *sqlite3_db) {
/*
	char query[4098];
	sprintf(query, "select id from movies_clean;");

	MYSQL_RES *res;
	MYSQL_ROW row;

	char query[4098];
	sprintf(query, "select id from movies_clean;");


	//  select sum(rating) / count(rating) as rate, count(rating) as numRate, cluster from ratings, profiles_Data_Allmovie where ratings.customer_id = profiles_Data_Allmovie.customer_id and movie_id = 8 group by cluster;



	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	int k = 0;

	if (numrows > 0) {
		while ((row = mysql_fetch_row(res)) != NULL) {

			if((k%10) == 0 || k == numrows)
				printf("%d of %d (%f%%) movies processed...\n", k, numrows, ((float)k/(float)numrows)*100);

			int m_id = atoi(row[0]);

			sprintf(query, "delete from ratings_noprobe where movie_id = %d and customer_id in (select customer_id from probe where movie_id = %d);",m_id, m_id);

			if (mysql_query(conn, query)) {
				fprintf(stderr, "%s\n", mysql_error(conn));
				exit(1);
			}

			k++;

//			res2 = mysql_store_result(conn);

//			int numrows2 = mysql_affected_rows(res2);

//			if(numrows2 < 1) {
//				fprintf(stderr, "No rows modified: %s\n", query);
//			}
		}
	}
*/
	return 0;

}

int fixProbe(profile_t *profile, int idMovie, float *rate, char *rateDb, MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	char query[4098];
	sprintf(query, "select rating from %s where movie_id = %d and customer_id = %d;",
			rateDb, idMovie, profile->id);

	/*
	 * select sum(rating) / count(rating) as rate, count(rating) as numRate, cluster from ratings, profiles_Data_Allmovie where ratings.customer_id = profiles_Data_Allmovie.customer_id and movie_id = 8 group by cluster;
	 */


	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	if (numrows > 0) {
		while ((row = mysql_fetch_row(res)) != NULL) {
			if(atoi(row[0]) > 0) {
				*rate = atof(row[0]);
			}
			else
				*rate = -1;
		}
	}
	else
		*rate = -1;

	mysql_free_result(res);

	return 0;
}



int getIds(int *ids, char *sqlite3_db) {
	sqlite3 *db;
	sqlite3_stmt *pStmt;

	if(sqlite3_open_v2(sqlite3_db, &db, SQLITE_OPEN_FULLMUTEX, NULL)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
	}

	char query[4098];

	int block = 10000;
	int x = 0;
	int off = 0;

//	sprintf(query, "select distinct customer_id from probe limit %d,%d;",x*block+off,block);
//	sprintf(query, "select distinct customer_id from probe where prediction = -2 order by customer_id limit %d,%d;",x*block+off,block);
	sprintf(query, "select distinct customer_id from probe order by customer_id limit %d,%d;",x*block+off,block);
//	sprintf(query, "select distinct customer_id from probe where rating = -1 limit %d,%d;",x*block+off,block);

	sqlite3_prepare_v2(db, query, -1, &pStmt, 0);
	int numrows = 0;
	while(sqlite3_step(pStmt)==SQLITE_ROW) {

		if((numrows%10000) == 0)
			printf("%d ids processed...\n", numrows);
//		int nColumn = sqlite3_column_count(pStmt);
		ids[numrows] = sqlite3_column_int(pStmt, 0);
		numrows++;
	}

	sqlite3_finalize(pStmt);
	sqlite3_close(db);

	return numrows;
}

int getMovieIds(profile_t *profile, int *movieIds, char *sqlite3_db) {
	sqlite3 *db;
	sqlite3_stmt *pStmt;

	if(sqlite3_open_v2(sqlite3_db, &db, SQLITE_OPEN_FULLMUTEX, NULL)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
	}

	char query[4098];
	sprintf(query, "select count(*) from probe where customer_id = %d;", profile->id);
	int numrows = 0;
	sqlite3_prepare_v2(db, query, -1, &pStmt, 0);
	if(sqlite3_step(pStmt)==SQLITE_ROW) {
		numrows = sqlite3_column_int(pStmt, 0);
	}
	if(numrows) {
		//movieIds = malloc(numrows*sizeof(int));

		sprintf(query, "select movie_id from probe where customer_id = %d;", profile->id);
		sqlite3_prepare_v2(db, query, -1, &pStmt, 0);
		int k = 0;
		while(sqlite3_step(pStmt)==SQLITE_ROW) {
			movieIds[k] = sqlite3_column_int(pStmt, 0);
			k++;
		}
//		printf("%d movies loaded...\n", k);
	}

	sqlite3_finalize(pStmt);
	sqlite3_close(db);

	return numrows;
}
