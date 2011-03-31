/*
 * ColaborativeRecommender.h
 *
 *  Created on: Nov 22, 2010
 *      Author: aislan
 */

#ifndef COLABORATIVERECOMMENDER_H_
#define COLABORATIVERECOMMENDER_H_

#include <mysql.h>

#include "SSACT.h"
#include "constants.h"

int getRate(profile_t *profile, int idMovie, float *rate, char *rateDb, char *movieDb, char *sqlite3_db);

#endif /* COLABORATIVERECOMMENDER_H_ */
