/*
 * SSACT.h
 *
 *  Created on: Aug 6, 2010
 *      Author: aislan
 */

#ifndef SSACT_H_
#define SSACT_H_

//#define USE_MYSQL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mysql.h>
#include <sqlite3.h>

#include "constants.h"
#include "ProfileGenerator.h"
#include "ClusterGenerator.h"
#include "SystemMaintence.h"

#ifdef USE_MYSQL
#include "Tools.h"
#include "ProfileAnalyses.h"

#else
#include "Tools_sqlite.h"
#include "ProfileAnalyses_sqlite.h"
#endif

typedef struct {
	int id;
	float genres[NUMBER_OF_GEN];
	int cluster;
} profile_t;

#endif /* SSACT_H_ */
