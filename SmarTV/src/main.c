/*
 * main.c
 *
 *  Created on: Aug 6, 2010
 *      Author: aislan
 */

//#define PROF_DB
//#define CLUSTER

#include "SSACT.h"

int main(void) {

// 	int ids[] = { 123126, 2118461, 1932594, 2143500, 1977959 };
 	int ids[] = { 123126, 2118461, 1932594, 2143500, 1977959, 1570292, 2482738, 676682, 307530, 1228542, 1404976, 2311335, 780341};

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

	/* send SQL query */
	if (mysql_query(conn, "show tables")) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_use_result(conn);

	/* output table name */
	printf("MySQL Tables in mysql database:\n");
	while ((row = mysql_fetch_row(res)) != NULL)
		printf("%s \n", row[0]);

	/* close connection */
	mysql_free_result(res);

#ifndef PROF_DB

	genProfile(0, "Allmovie", conn);
//	genProfile(0, "IMDb", conn);

#else

	char query[1024];
	sprintf(query, "select distinct customer_id from ratings order by customer_id asc;");
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	while ((row = mysql_fetch_row(res)) != NULL) {
 		genProfileAllmovie(atoi(row[0]), conn);
 		genProfileIMDB(atoi(row[0]), conn);
 	}

#endif
#ifdef CLUSTER

	genCluster("profiles_RateCntPer", conn);

#endif

	mysql_close(conn);

	return EXIT_SUCCESS;
}
