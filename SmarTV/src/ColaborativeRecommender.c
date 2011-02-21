/*
 * ColaborativeRecommender.c
 *
 *  Created on: Nov 22, 2010
 *      Author: aislan
 */

#include "ColaborativeRecommender.h"

int getRate(profile_t *profile, int idMovie, float *rate, char *rateDb, char *movieDb, MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	char query[4098];
	sprintf(query, "select sum(rating) / count(rating) as meanRate, count(rating) as countRate from %s where movie_id = %d "
			"and status = 1 and customer_id in "
			"(select customer_id from profiles_Data_%s where cluster = %d);",
			rateDb, idMovie, movieDb, profile->cluster);

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
			if(atoi(row[1]) > 0) {
				*rate = atof(row[0]);
			}
			else
				*rate = -2;
		}
	}
	else
		*rate = -3;

	mysql_free_result(res);

	return 0;
}
