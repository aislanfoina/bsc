/*
 * ProfileGenerator.h
 *
 *  Created on: Aug 5, 2010
 *      Author: aislan
 */

#ifndef PROFILEGENERATOR_H_
#define PROFILEGENERATOR_H_

#define SAVE

#define MAX_ROW_CNT 100000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mysql.h>

#include "constants.h"


int translateGenreId(char *genreName);
int translateIdGenre(int genreId, char *genreName);
int translateGenreIdAllmovie(char *genreName);

int genProfile(int userId, char *movieDb, MYSQL *conn);
int genProfileIMDB(int userId, MYSQL *conn);
int genProfileAllmovie(int userId, MYSQL *conn);

int saveIntProfile(int userId, int *profile, char *table, char *moviedb, MYSQL *conn);
int saveFloatProfile(int userId, float *profile, char *table, char *moviedb, MYSQL *conn);
int saveDataProfile(int userId, int totalRates, float meanRate, float stdRate, char *table, char *moviedb, MYSQL *conn);

#endif /* PROFILEGENERATOR_H_ */
