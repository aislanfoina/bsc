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

int saveProfile(float *profile, int userId, FILE *fp) {
	int k;

	for (k = 0; k < NUMBER_OF_GEN; k++) {
		fprintf(fp, "%f,",profile[k]);
	}

	fprintf(fp,"%d\n", userId);

	return 0;
}

int saveCluster (char *table, MYSQL *conn) {
	int k;

	MYSQL_RES *res;
	MYSQL_ROW row;

	/* send SQL query */
	char query[4098];
	sprintf(query, "select customer_id from %s "
			"where customer_id != 0 "
			"order by customer_id asc;", table);
	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n%s\n", mysql_error(conn), query);
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	char *path = "clusters_allmovie.csv";
	int MAX_PROF_NUM = numrows;
	float *prof_clustered;

	prof_clustered = malloc(MAX_PROF_NUM*(NUMBER_OF_GEN+1)*sizeof(float));

	getDataset(path, prof_clustered, MAX_PROF_NUM, NUMBER_OF_GEN+1);

	k = 0;

	while ((row = mysql_fetch_row(res)) != NULL) {
		MYSQL_RES *res2;

		sprintf(query, "update %s "
				"set cluster = %d "
				"where customer_id = %d;", table,
					(int) prof_clustered[k*(NUMBER_OF_GEN+1)+NUMBER_OF_GEN],
					(int) atoi(row[0]));

		if (mysql_query(conn, query)) {
			fprintf(stderr, "%s\n%s\n", mysql_error(conn), query);
			exit(1);
		}

		k++;

		if((k%1000) == 0 || k == numrows)
			printf("%d of %d (%f%%) profiles processed...\n", k, numrows, ((float)k/(float)numrows)*100);

		res2 = mysql_store_result(conn);
		int numrows2 = mysql_num_rows(res2);

		if(numrows2 < 1) {
			fprintf(stderr, "No record updated!\n%s\n", query);
		}

	}

	return 0;
}

