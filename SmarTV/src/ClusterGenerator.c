/*
 ============================================================================
 Name        : ClusterGenerator.c
 Author      : Aislan G. Foina
 Version     :
 Copyright   : Your copyright notice
 Description : Gerador de Clusters dos Perfis
 ============================================================================
 */

#include "ClusterGenerator.h"

int genCluster(char *table, MYSQL *conn) {

	MYSQL_RES *res;
	MYSQL_ROW row;

	int numgens = NUMBER_OF_GEN;

	/* send SQL query */
	char query[4098];
	sprintf(query, "select Action, Adult, Adventure, Animation, Biography, "
			"Comedy, Crime, Documentary, Drama, Family, "
			"Fantasy, FilmNoir, GameShow, History, Horror, "
			"Music, Musical, Mystery, News, RealityTV, "
			"Romance, SciFi, Short, Sport, TalkShow, "
			"Thriller, War, Western, "
			"customer_id from %s "
			"order by customer_id asc;", table);
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n%s\n", mysql_error(conn), query);
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	if (numrows > 0) {
		/* output table name */
		printf("%d user profiles returned from [%s]:\n", numrows, table);

		float profile[numgens+1];

		int k;
		FILE *fp;

	    fp = fopen("profiles.csv","w");
	    if ( fp == NULL ) {
	           printf("Cannot open gmeans csv file\n");
	           exit(1);
	    }

	    while ((row = mysql_fetch_row(res)) != NULL) {
	    	memset(profile, 0, numgens * sizeof(float));
	    	for(k = 0; k < numgens + 1; k++)
				profile[k] = (float) atof(row[k]);

			saveProfile(profile, profile[numgens], fp);
		}
	    fclose (fp);


		printf("\nEnd of table %s processing\n\n\n", table);
		/* close connection */
		mysql_free_result(res);
	}
	return 0;
}

int saveProfile(float *profile, int userId, FILE *fp){
	int k;

	for (k = 0; k < NUMBER_OF_GEN; k++) {
		fprintf(fp, "%f,",profile[k]);
	}

	fprintf(fp,"%d\n", userId);

	return 0;
}

