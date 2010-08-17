/*
 * SystemMaintence.c
 *
 *  Created on: Aug 17, 2010
 *      Author: aislan
 */

#include "SystemMaintence.h"

void updateProfilesTable(MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	char query[1024];
	sprintf(query, "select distinct customer_id from ratings order by customer_id asc;");
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

//	int numrows = mysql_num_rows(res);

	while ((row = mysql_fetch_row(res)) != NULL) {
 		genProfile(atoi(row[0]), "Allmovie", conn);
 		genProfile(atoi(row[0]), "IMDb", conn);
 	}
}

void calculateClusters(MYSQL *conn) {
	MYSQL_RES *res;
	MYSQL_ROW row;

	int numrec = genCluster("profiles_RateCntPer_Allmovie", conn);

	// call gmeans;
}
