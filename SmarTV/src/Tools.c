/*
 * ColaborativeRecommender.c
 *
 *  Created on: Nov 22, 2010
 *      Author: aislan
 */

#include "Tools.h"

int cleanRatings(MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	char query[4098];
	sprintf(query, "select id from movies_clean;");

	/*
	 * select sum(rating) / count(rating) as rate, count(rating) as numRate, cluster from ratings, profiles_Data_Allmovie where ratings.customer_id = profiles_Data_Allmovie.customer_id and movie_id = 8 group by cluster;
	 */


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

	return 0;
}

int getIds(int *ids, MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	char query[4098];

	int block = 10000;
	int x = 0;
	int off = 7280;
	sprintf(query, "select distinct customer_id from probe limit %d,%d;",x*block+off,block);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	int k = 0;

	if (numrows > 0) {

//		ids = malloc(numrows*sizeof(int));

		while ((row = mysql_fetch_row(res)) != NULL) {

			if((k%10000) == 0 || k == numrows)
				printf("%d of %d (%f%%) ids processed...\n", k, numrows, ((float)k/(float)numrows)*100);

			ids[k] = atoi(row[0]);

			k++;

		}
	}

	return numrows;
}

int getMovieIds(profile_t *profile, int *movieIds, MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	char query[4098];
	sprintf(query, "select movie_id from probe where customer_id = %d;", profile->id);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	int k = 0;

	if (numrows > 0) {

//		movieIds = malloc(numrows*sizeof(int));

		while ((row = mysql_fetch_row(res)) != NULL) {

			movieIds[k] = atoi(row[0]);

			k++;

		}

//		printf("%d movies loaded...\n", k);

	}

	return numrows;
}
