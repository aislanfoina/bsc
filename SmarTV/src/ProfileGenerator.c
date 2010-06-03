/*
 ============================================================================
 Name        : ProfileGenerator.c
 Author      : Aislan G. Foina
 Version     :
 Copyright   : Your copyright notice
 Description : Gerador de Perfil de Usuario
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mysql.h>

int main(void) {

 	int ids[] = { 123126, 2118461, 1932594, 2143500, 1977959 };

 	int numgens = 28;

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

	int i;
	for (i = 0; i < (sizeof(ids)/sizeof(int)); i++) {
		int id = ids[i];

		/* send SQL query */
		char query[1024];
		sprintf(query,"select rating, genre, date from ratings, imdbgenres "
				"where customer_id = %d and ratings.movie_id = imdbgenres.netflixid "
				"order by date desc;", id);
		if (mysql_query(conn, query)) {
			fprintf(stderr, "%s\n", mysql_error(conn));
			exit(1);
		}

		res = mysql_store_result(conn);

		int numrows = mysql_num_rows(res);

		if (numrows > 0) {
			/* output table name */
			printf("%d ratings returned for the user [%d]:\n",numrows,id);

			float totalMean = 0;
			float totalStd = 0;
			int totalCount = 0;

			int genCounter[numgens];
			memset(genCounter, 0, numgens*sizeof(int));
			int genPosCounter[numgens];
			memset(genPosCounter, 0, numgens*sizeof(int));
			int genNegCounter[numgens];
			memset(genNegCounter, 0, numgens*sizeof(int));
			int genRate[numgens];
			memset(genRate, 0, numgens*sizeof(int));
			float genMean[numgens];
			memset(genMean, 0, numgens*sizeof(float));
			float genStd[numgens];
			memset(genStd, 0, numgens*sizeof(float));

			while ((row = mysql_fetch_row(res)) != NULL) {
				int rate = atoi(row[0]);
				int genId = translateGenreId(row[1]);

				if(genId == -1) {
					fprintf(stderr, "Impossible to translate Genre Name %s\n", row[1]);
					exit(1);
				}

				genCounter[genId]++;
				genRate[genId]+=rate;
				genStd[genId]+=rate*rate;

				totalCount++;
				totalMean+=rate;
				totalStd+=rate*rate;

				if(rate > 3)
					genPosCounter[genId]++;
				else if(rate < 3)
					genNegCounter[genId]++;
			}
			int k;

			for (k = 0; k < numgens; k++)
				if(genCounter[k] != 0) {
					genMean[k] = (float) genRate[k]/ (float) genCounter[k];
					genStd[k] = sqrt((genStd[k]/ (float) genCounter[k])-(genMean[k]*genMean[k]));
				}
			totalMean = totalMean/(float) totalCount;
			totalStd = sqrt((totalStd/totalCount)-(totalMean*totalMean));

			printf("Genres: \t");
			for (k = 0; k < numgens; k++) {
				char *genreName;
				genreName = (char *) malloc (64 * sizeof(char));

				if (translateIdGenre(k, genreName) == -1) {
					fprintf(stderr, "Impossible to translate Genre ID %s\n", k);
					exit(1);
				}
				printf("%s\t",genreName);
			}
			printf("\nRateCounter: \t");
			for (k = 0; k < numgens; k++)
				printf("%d\t",genCounter[k]);

			printf("\nRatePosCounter: \t");
			for (k = 0; k < numgens; k++)
				printf("%d\t",genPosCounter[k]);

			printf("\nRateNegCounter: \t");
			for (k = 0; k < numgens; k++)
				printf("%d\t",genNegCounter[k]);

			printf("\nRatePosMinusNegCounter: \t");
			for (k = 0; k < numgens; k++)
				printf("%d\t",genPosCounter[k]-genNegCounter[k]);

			printf("\nMeanRate: \t");
			for (k = 0; k < numgens; k++)
				printf("%f\t",genMean[k]);

			printf("\nSTDRate: \t");
			for (k = 0; k < numgens; k++)
				printf("%f\t",genStd[k]);

			printf("\nNormalizedRate: \t");
			for (k = 0; k < numgens; k++)
				printf("%f\t",(genMean[k]-totalMean)/totalStd);

			printf("\n\n");

			printf("TotalCount: \t %d\n",totalCount);
			printf("TotalMean: \t %f\n",totalMean);
			printf("TotalStd: \t %f\n",totalStd);

			printf("\nEnd of user %d processing\n",id);
			/* close connection */
			mysql_free_result(res);
		}

	}

	mysql_close(conn);

	return EXIT_SUCCESS;
}

int translateGenreId(char *genreName) {
	if(!strcmp(genreName,"Action"))
		return 0;
	else if(!strcmp(genreName,"Adult"))
		return 27;
	else if(!strcmp(genreName,"Adventure"))
		return 1;
	else if(!strcmp(genreName,"Animation"))
		return 2;
	else if(!strcmp(genreName,"Biography"))
		return 3;
	else if(!strcmp(genreName,"Comedy"))
		return 4;
	else if(!strcmp(genreName,"Crime"))
		return 5;
	else if(!strcmp(genreName,"Documentary"))
		return 6;
	else if(!strcmp(genreName,"Drama"))
		return 7;
	else if(!strcmp(genreName,"Family"))
		return 8;
	else if(!strcmp(genreName,"Fantasy"))
		return 9;
	else if(!strcmp(genreName,"Film-Noir"))
		return 10;
	else if(!strcmp(genreName,"Game-Show"))
		return 11;
	else if(!strcmp(genreName,"History"))
		return 12;
	else if(!strcmp(genreName,"Horror"))
		return 13;
	else if(!strcmp(genreName,"Music"))
		return 14;
	else if(!strcmp(genreName,"Musical"))
		return 15;
	else if(!strcmp(genreName,"Mystery"))
		return 16;
	else if(!strcmp(genreName,"News"))
		return 17;
	else if(!strcmp(genreName,"Reality-TV"))
		return 18;
	else if(!strcmp(genreName,"Romance"))
		return 19;
	else if(!strcmp(genreName,"Sci-Fi"))
		return 20;
	else if(!strcmp(genreName,"Short"))
		return 21;
	else if(!strcmp(genreName,"Sport"))
		return 22;
	else if(!strcmp(genreName,"Talk-Show"))
		return 23;
	else if(!strcmp(genreName,"Thriller"))
		return 24;
	else if(!strcmp(genreName,"War"))
		return 25;
	else if(!strcmp(genreName,"Western"))
		return 26;
	else
		return -1;
}

int translateIdGenre(int genreId, char *genreName) {
	char genre[64];

	if(genreId == 0)
		strcpy(genre,"Action");
	else if(genreId == 27)
		strcpy(genre,"Adult");
	else if(genreId == 1)
		strcpy(genre,"Adventure");
	else if(genreId == 2)
		strcpy(genre,"Animation");
	else if(genreId == 3)
		strcpy(genre,"Biography");
	else if(genreId == 4)
		strcpy(genre,"Comedy");
	else if(genreId == 5)
		strcpy(genre,"Crime");
	else if(genreId == 6)
		strcpy(genre,"Documentary");
	else if(genreId == 7)
		strcpy(genre,"Drama");
	else if(genreId == 8)
		strcpy(genre,"Family");
	else if(genreId == 9)
		strcpy(genre,"Fantasy");
	else if(genreId == 10)
		strcpy(genre,"Film-Noir");
	else if(genreId == 11)
		strcpy(genre,"Game-Show");
	else if(genreId == 12)
		strcpy(genre,"History");
	else if(genreId == 13)
		strcpy(genre,"Horror");
	else if(genreId == 14)
		strcpy(genre,"Music");
	else if(genreId == 15)
		strcpy(genre,"Musical");
	else if(genreId == 16)
		strcpy(genre,"Mystery");
	else if(genreId == 17)
		strcpy(genre,"News");
	else if(genreId == 18)
		strcpy(genre,"Reality-TV");
	else if(genreId == 19)
		strcpy(genre,"Romance");
	else if(genreId == 20)
		strcpy(genre,"Sci-Fi");
	else if(genreId == 21)
		strcpy(genre,"Short");
	else if(genreId == 22)
		strcpy(genre,"Sport");
	else if(genreId == 23)
		strcpy(genre,"Talk-Show");
	else if(genreId == 24)
		strcpy(genre,"Thriller");
	else if(genreId == 25)
		strcpy(genre,"War");
	else if(genreId == 26)
		strcpy(genre,"Western");
	else {
		strcpy(genre,"Invalid ID!");
		return -1;
	}

	strcpy(genreName, genre);
	return 1;
}
