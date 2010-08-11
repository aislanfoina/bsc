/*
 ============================================================================
 Name        : ProfileGenerator.c
 Author      : Aislan G. Foina
 Version     :
 Copyright   : Your copyright notice
 Description : Gerador de Perfil de Usuario
 ============================================================================
 */

#include "ProfileGenerator.h"

int genProfileIMDB(int userId, MYSQL *conn) {
	int numgens = NUMBER_OF_GEN;

	MYSQL_RES *res;
	MYSQL_ROW row;

	int id = userId;

	/* send SQL query */
	char query[4098];
	sprintf(query, "select rating, genre, date from ratings, imdbgenres "
			"where ratings.movie_id = imdbgenres.netflixid");
	if(userId) {
		sprintf(query, "%s and customer_id = %d",query,id);
	}
	sprintf(query, "%s order by date desc;",query);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	if (numrows > 20) {
		/* output table name */
		printf("%d ratings returned for the user [%d]:\n", numrows, id);

		float totalMean = 0;
		float totalStd = 0;
		int totalCount = 0;

		int genCounter[numgens];
		memset(genCounter, 0, numgens * sizeof(int));

		int gen5Counter[numgens];
		memset(gen5Counter, 0, numgens * sizeof(int));
		int gen4Counter[numgens];
		memset(gen4Counter, 0, numgens * sizeof(int));
		int gen3Counter[numgens];
		memset(gen3Counter, 0, numgens * sizeof(int));
		int gen2Counter[numgens];
		memset(gen2Counter, 0, numgens * sizeof(int));
		int gen1Counter[numgens];
		memset(gen1Counter, 0, numgens * sizeof(int));

		int genPosCounter[numgens];
		memset(genPosCounter, 0, numgens * sizeof(int));
		int genMidCounter[numgens];
		memset(genMidCounter, 0, numgens * sizeof(int));
		int genNegCounter[numgens];
		memset(genNegCounter, 0, numgens * sizeof(int));

		float genRate[numgens];
		memset(genRate, 0, numgens * sizeof(float));
		float genPosRate[numgens];
		memset(genPosRate, 0, numgens * sizeof(float));
		float genNegRate[numgens];
		memset(genNegRate, 0, numgens * sizeof(float));
		float genMidRate[numgens];
		memset(genMidRate, 0, numgens * sizeof(float));

		float genMean[numgens];
		memset(genMean, 0, numgens * sizeof(float));
		float genStd[numgens];
		memset(genStd, 0, numgens * sizeof(float));

		while ((row = mysql_fetch_row(res)) != NULL) {
			int origrate = atoi(row[0]);
			float rate = (float) (origrate - 3) / (float) 2;

			int genId = translateGenreId(row[1]);

			if (genId == -1) {
				fprintf(stderr, "Impossible to translate Genre Name %s\n",
						row[1]);
				exit(1);
			}

			switch (origrate) {
			case 5:
				gen5Counter[genId]++;
				break;
			case 4:
				gen4Counter[genId]++;
				break;
			case 3:
				gen3Counter[genId]++;
				break;
			case 2:
				gen2Counter[genId]++;
				break;
			case 1:
				gen1Counter[genId]++;
				break;
			}

			if (rate > 0) {
				genPosCounter[genId]++;
				genPosRate[genId] += rate;
			} else if (rate < 0) {
				genNegCounter[genId]++;
				genNegRate[genId] += rate;
			} else {
				genMidCounter[genId]++;
				genMidRate[genId] += rate;
			}

			genCounter[genId]++;
			genRate[genId] += rate;
			genStd[genId] += rate * rate;

			totalCount++;
			totalMean += rate;
			totalStd += rate * rate;

		}
		int k;

		for (k = 0; k < numgens; k++) {
			if (genCounter[k] != 0) {
				genMean[k] = (float) genRate[k] / (float) genCounter[k];
				genStd[k] = sqrt((genStd[k] / (float) genCounter[k])
						- (genMean[k] * genMean[k]));
			}
		}
		float totalMean2 = 0;
		float totalStd2 = 0;
		for (k = 0; k < numgens; k++) {
			totalMean2 += genMean[k];
			totalStd2 += genMean[k]*genMean[k];
		}
		totalMean2 = totalMean2/numgens;
		totalStd2 = sqrt((totalStd2 / numgens) - (totalMean2 * totalMean2));

		totalMean = totalMean / (float) totalCount;
		totalStd = sqrt((totalStd / totalCount) - (totalMean * totalMean));

		printf("Genres: \t");
		for (k = 0; k < numgens; k++) {
			char *genreName;
			genreName = (char *) malloc(64 * sizeof(char));

			if (translateIdGenre(k, genreName) == -1) {
				fprintf(stderr, "Impossible to translate Genre ID %d\n", k);
				exit(1);
			}
			printf("%s\t", genreName);
		}
		printf("\nRateCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genCounter[k]);

		printf("\nRatePosCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genPosCounter[k]);

		printf("\nRateNegCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genNegCounter[k]);

		printf("\nRateMidCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genMidCounter[k]);

		printf("\nDon't Care: \t");
		for (k = 0; k < numgens; k++) {
			float dont_care = (float) genPosCounter[k]
					/ (float) genNegCounter[k];
			if (dont_care > 1)
				dont_care = (float) 1 / dont_care;
			if (dont_care > TH_DONT_CARE)
				printf("* %f\t", dont_care);
			else
				printf("%f\t", dont_care);
		}
		printf("\nDon't Care2: \t");
		for (k = 0; k < numgens; k++) {
			float dont_care = (float) genMidCounter[k]
					/ (float) (genPosCounter[k] + genNegCounter[k]);
			if (dont_care > 1)
				dont_care = (float) 1 / dont_care;
			if (dont_care > TH_DONT_CARE)
				printf("* %f\t", dont_care);
			else
				printf("%f\t", dont_care);
		}
		printf("\nHigh Pos: \t");
		for (k = 0; k < numgens; k++) {
			float high_pos = (float) genPosCounter[k]
					/ (float) (genPosCounter[k] + genMidCounter[k]
							+ genNegCounter[k]);
			if (high_pos > TH_HIGH_RATIO)
				printf("* %f\t", high_pos);
			else
				printf("%f\t", high_pos);
		}

		printf("\nHigh Mid: \t");
		for (k = 0; k < numgens; k++) {
			float high_pos = (float) genMidCounter[k]
					/ (float) (genPosCounter[k] + genMidCounter[k]
							+ genNegCounter[k]);
			if (high_pos > TH_HIGH_RATIO)
				printf("* %f\t", high_pos);
			else
				printf("%f\t", high_pos);
		}

		printf("\nHigh Neg: \t");
		for (k = 0; k < numgens; k++) {
			float high_neg = (float) genNegCounter[k]
					/ (float) (genPosCounter[k] + genMidCounter[k]
							+ genNegCounter[k]);
			if (high_neg > TH_HIGH_RATIO)
				printf("* %f\t", high_neg);
			else
				printf("%f\t", high_neg);
		}

		printf("\nRatePosMinusNegCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genPosCounter[k] - genNegCounter[k]);

		printf("\nMeanRate: \t");
		for (k = 0; k < numgens; k++)
			printf("%f\t", genMean[k]);

		printf("\nSTDRate: \t");
		for (k = 0; k < numgens; k++)
			printf("%f\t", genStd[k]);

		printf("\nNormalizedRate: \t");
		for (k = 0; k < numgens; k++)
			printf("%f\t", (genMean[k] - totalMean) / totalStd);

		printf("\nNormalizedRate2: \t");
		for (k = 0; k < numgens; k++)
			printf("%f\t", (genMean[k] - totalMean2) / totalStd2);

		printf("\n\n");

		printf("TotalCount: \t %d\n", totalCount);
		printf("TotalMean: \t %f\n", totalMean);
		printf("TotalStd: \t %f\n", totalStd);

		printf("TotalMean2: \t %f\n", totalMean2);
		printf("TotalStd2: \t %f\n", totalStd2);

		printf("\nEnd of user %d processing\n\n\n", id);
		/* close connection */
		mysql_free_result(res);

#ifdef SAVE
/*
 *
 * Data save
 *
 */
		saveDataProfile(id, totalCount, totalMean2, totalStd2, "profiles_Data_IMDb", conn);
		saveIntProfile(id, genCounter, "profiles_RateCounter_IMDb", conn);


		float genCntPer[numgens];
		memset(genCntPer, 0, numgens * sizeof(float));

		for(k=0; k < numgens; k++) {
			genCntPer[k] = (float)genCounter[k]/(float)totalCount;
		}

		saveFloatProfile(id, genCntPer, "profiles_RateCntPer_IMDb", conn);

		float genNormal[numgens];
		memset(genNormal, 0, numgens * sizeof(float));

		if(!isnan(totalStd2) && totalStd2 != 0) {
			for(k=0; k < numgens; k++) {
				genNormal[k] = (float)(genMean[k] - totalMean2) / (float)totalStd2;
			}
		}

		saveFloatProfile(id, genNormal, "profiles_RateNormal_IMDb", conn);
#endif
	}
	return 0;
}


int genProfileAllmovie(int userId, MYSQL *conn) {
	int numgens = NUMBER_OF_GEN;

	MYSQL_RES *res;
	MYSQL_ROW row;

	int id = userId;

	/* send SQL query */
	char query[4098];
	sprintf(query, "select rating, genre, date from ratings, allmoviegenres "
			"where ratings.movie_id = allmoviegenres.netflixid");
	if(userId) {
		sprintf(query, "%s and customer_id = %d",query,id);
	}
	sprintf(query, "%s order by date desc;",query);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	int numrows = mysql_num_rows(res);

	if (numrows > 20) {
		/* output table name */
		printf("%d ratings returned for the user [%d]:\n", numrows, id);

		float totalMean = 0;
		float totalStd = 0;
		int totalCount = 0;

		int genCounter[numgens];
		memset(genCounter, 0, numgens * sizeof(int));

		int gen5Counter[numgens];
		memset(gen5Counter, 0, numgens * sizeof(int));
		int gen4Counter[numgens];
		memset(gen4Counter, 0, numgens * sizeof(int));
		int gen3Counter[numgens];
		memset(gen3Counter, 0, numgens * sizeof(int));
		int gen2Counter[numgens];
		memset(gen2Counter, 0, numgens * sizeof(int));
		int gen1Counter[numgens];
		memset(gen1Counter, 0, numgens * sizeof(int));

		int genPosCounter[numgens];
		memset(genPosCounter, 0, numgens * sizeof(int));
		int genMidCounter[numgens];
		memset(genMidCounter, 0, numgens * sizeof(int));
		int genNegCounter[numgens];
		memset(genNegCounter, 0, numgens * sizeof(int));

		float genRate[numgens];
		memset(genRate, 0, numgens * sizeof(float));
		float genPosRate[numgens];
		memset(genPosRate, 0, numgens * sizeof(float));
		float genNegRate[numgens];
		memset(genNegRate, 0, numgens * sizeof(float));
		float genMidRate[numgens];
		memset(genMidRate, 0, numgens * sizeof(float));

		float genMean[numgens];
		memset(genMean, 0, numgens * sizeof(float));
		float genStd[numgens];
		memset(genStd, 0, numgens * sizeof(float));

		while ((row = mysql_fetch_row(res)) != NULL) {
			int origrate = atoi(row[0]);
			float rate = (float) (origrate - 3) / (float) 2;

			int genId = translateGenreIdAllmovie(row[1]);

			if (genId < 0) {
//				fprintf(stderr, "Impossible to translate Genre Name %s\n",
//						row[1]);
			}
			else {

				switch (origrate) {
				case 5:
					gen5Counter[genId]++;
					break;
				case 4:
					gen4Counter[genId]++;
					break;
				case 3:
					gen3Counter[genId]++;
					break;
				case 2:
					gen2Counter[genId]++;
					break;
				case 1:
					gen1Counter[genId]++;
					break;
				}

				if (rate > 0) {
					genPosCounter[genId]++;
					genPosRate[genId] += rate;
				} else if (rate < 0) {
					genNegCounter[genId]++;
					genNegRate[genId] += rate;
				} else {
					genMidCounter[genId]++;
					genMidRate[genId] += rate;
				}

				genCounter[genId]++;
				genRate[genId] += rate;
				genStd[genId] += rate * rate;

				totalCount++;
				totalMean += rate;
				totalStd += rate * rate;
			}
		}
		int k;

		for (k = 0; k < numgens; k++) {
			if (genCounter[k] != 0) {
				genMean[k] = (float) genRate[k] / (float) genCounter[k];
				genStd[k] = sqrt((genStd[k] / (float) genCounter[k])
						- (genMean[k] * genMean[k]));
			}
		}
		float totalMean2 = 0;
		float totalStd2 = 0;
		for (k = 0; k < numgens; k++) {
			totalMean2 += genMean[k];
			totalStd2 += genMean[k]*genMean[k];
		}
		totalMean2 = totalMean2/numgens;
		totalStd2 = sqrt((totalStd2 / numgens) - (totalMean2 * totalMean2));

		totalMean = totalMean / (float) totalCount;
		totalStd = sqrt((totalStd / totalCount) - (totalMean * totalMean));

		printf("Genres: \t");
		for (k = 0; k < numgens; k++) {
			char *genreName;
			genreName = (char *) malloc(64 * sizeof(char));

			if (translateIdGenre(k, genreName) == -1) {
				fprintf(stderr, "Impossible to translate Genre ID %d\n", k);
				exit(1);
			}
			printf("%s\t", genreName);
		}
		printf("\nRateCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genCounter[k]);

		printf("\nRatePosCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genPosCounter[k]);

		printf("\nRateNegCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genNegCounter[k]);

		printf("\nRateMidCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genMidCounter[k]);

		printf("\nDon't Care: \t");
		for (k = 0; k < numgens; k++) {
			float dont_care = (float) genPosCounter[k]
					/ (float) genNegCounter[k];
			if (dont_care > 1)
				dont_care = (float) 1 / dont_care;
			if (dont_care > TH_DONT_CARE)
				printf("* %f\t", dont_care);
			else
				printf("%f\t", dont_care);
		}
		printf("\nDon't Care2: \t");
		for (k = 0; k < numgens; k++) {
			float dont_care = (float) genMidCounter[k]
					/ (float) (genPosCounter[k] + genNegCounter[k]);
			if (dont_care > 1)
				dont_care = (float) 1 / dont_care;
			if (dont_care > TH_DONT_CARE)
				printf("* %f\t", dont_care);
			else
				printf("%f\t", dont_care);
		}
		printf("\nHigh Pos: \t");
		for (k = 0; k < numgens; k++) {
			float high_pos = (float) genPosCounter[k]
					/ (float) (genPosCounter[k] + genMidCounter[k]
							+ genNegCounter[k]);
			if (high_pos > TH_HIGH_RATIO)
				printf("* %f\t", high_pos);
			else
				printf("%f\t", high_pos);
		}

		printf("\nHigh Mid: \t");
		for (k = 0; k < numgens; k++) {
			float high_pos = (float) genMidCounter[k]
					/ (float) (genPosCounter[k] + genMidCounter[k]
							+ genNegCounter[k]);
			if (high_pos > TH_HIGH_RATIO)
				printf("* %f\t", high_pos);
			else
				printf("%f\t", high_pos);
		}

		printf("\nHigh Neg: \t");
		for (k = 0; k < numgens; k++) {
			float high_neg = (float) genNegCounter[k]
					/ (float) (genPosCounter[k] + genMidCounter[k]
							+ genNegCounter[k]);
			if (high_neg > TH_HIGH_RATIO)
				printf("* %f\t", high_neg);
			else
				printf("%f\t", high_neg);
		}

		printf("\nRatePosMinusNegCounter: \t");
		for (k = 0; k < numgens; k++)
			printf("%d\t", genPosCounter[k] - genNegCounter[k]);

		printf("\nMeanRate: \t");
		for (k = 0; k < numgens; k++)
			printf("%f\t", genMean[k]);

		printf("\nSTDRate: \t");
		for (k = 0; k < numgens; k++)
			printf("%f\t", genStd[k]);

		printf("\nNormalizedRate: \t");
		for (k = 0; k < numgens; k++)
			printf("%f\t", (genMean[k] - totalMean) / totalStd);

		printf("\nNormalizedRate2: \t");
		for (k = 0; k < numgens; k++)
			printf("%f\t", (genMean[k] - totalMean2) / totalStd2);

		printf("\n\n");

		printf("TotalCount: \t %d\n", totalCount);
		printf("TotalMean: \t %f\n", totalMean);
		printf("TotalStd: \t %f\n", totalStd);

		printf("TotalMean2: \t %f\n", totalMean2);
		printf("TotalStd2: \t %f\n", totalStd2);

		printf("\nEnd of user %d processing\n\n\n", id);
		/* close connection */
		mysql_free_result(res);

#ifdef SAVE

/*
 *
 * Data save
 *
 */

		saveDataProfile(id, totalCount, totalMean2, totalStd2, "profiles_Data_Allmovie", conn);
		saveIntProfile(id, genCounter, "profiles_RateCounter_Allmovie", conn);


		float genCntPer[numgens];
		memset(genCntPer, 0, numgens * sizeof(float));

		for(k=0; k < numgens; k++) {
			genCntPer[k] = (float)genCounter[k]/(float)totalCount;
		}

		saveFloatProfile(id, genCntPer, "profiles_RateCntPer_Allmovie", conn);

		float genNormal[numgens];
		memset(genNormal, 0, numgens * sizeof(float));

		if(!isnan(totalStd2) && totalStd2 != 0) {
			for(k=0; k < numgens; k++) {
				genNormal[k] = (float)(genMean[k] - totalMean2) / (float)totalStd2;
			}
		}

		saveFloatProfile(id, genNormal, "profiles_RateNormal_Allmovie", conn);

#endif

	}
	return 0;
}

int saveFloatProfile(int userId, float *profile, char *table, MYSQL *conn) {
	MYSQL_RES *res;
//	MYSQL_ROW row;

	char query[4098];

	sprintf(query,"insert into %s values (%d,"
			"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,"
			"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,"
			"%f,%f,%f,%f,%f,%f,%f,%f);",table,userId,
			profile[0],profile[1],profile[2],profile[3],profile[4],
			profile[5],profile[6],profile[7],profile[8],profile[9],
			profile[10],profile[11],profile[12],profile[13],profile[14],
			profile[15],profile[16],profile[17],profile[18],profile[19],
			profile[20],profile[21],profile[22],profile[23],profile[24],
			profile[25],profile[26],profile[27]);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "Insert %s: %s\n", table, mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);
	mysql_free_result(res);

	return 0;
}

int saveIntProfile(int userId, int *profile, char *table, MYSQL *conn) {
	MYSQL_RES *res;
//	MYSQL_ROW row;

	char query[4098];

	sprintf(query,"insert into %s values (%d,"
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
			"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
			"%d,%d,%d,%d,%d,%d,%d,%d);",table,userId,
			profile[0],profile[1],profile[2],profile[3],profile[4],
			profile[5],profile[6],profile[7],profile[8],profile[9],
			profile[10],profile[11],profile[12],profile[13],profile[14],
			profile[15],profile[16],profile[17],profile[18],profile[19],
			profile[20],profile[21],profile[22],profile[23],profile[24],
			profile[25],profile[26],profile[27]);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "Insert %s: %s\n", table, mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);
	mysql_free_result(res);

	return 0;
}

int saveDataProfile(int userId, int totalRates, float meanRate, float stdRate, char *table, MYSQL *conn) {
	MYSQL_RES *res;
//	MYSQL_ROW row;

	char query[4098];

	sprintf(query,"insert into profiles_Data values (%d,"
			"%d,%f,%f);",userId,
			totalRates, meanRate, stdRate);

	if (mysql_query(conn, query)) {
		fprintf(stderr, "Insert profiles_Data: %s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);
	mysql_free_result(res);

	return 0;
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

int translateGenreIdAllmovie(char *genreName) {
	if(!strcmp(genreName,"Action"))
		return 0;
	else if(!strcmp(genreName,"Adult"))
		return 27;
	else if(!strcmp(genreName,"Adventure"))
		return 1;
	else if(!strcmp(genreName,"Animated"))
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
