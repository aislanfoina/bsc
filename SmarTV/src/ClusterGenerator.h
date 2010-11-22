/*
 * ClusterGenerator.h
 *
 *  Created on: Aug 6, 2010
 *      Author: aislan
 */

#ifndef CLUSTERGENERATOR_H_
#define CLUSTERGENERATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mysql.h>

#include "constants.h"
#include "csv_parser.h"

int genCluster(char *table, MYSQL *conn);
int saveProfile(float *profile, int userId, FILE *fp);

#endif /* CLUSTERGENERATOR_H_ */
