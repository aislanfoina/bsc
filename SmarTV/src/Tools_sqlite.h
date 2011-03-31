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

int cleanRatings(char *sqlite3_db);
int getIds(int *ids, char *sqlite3_db);

#endif /* COLABORATIVERECOMMENDER_H_ */
