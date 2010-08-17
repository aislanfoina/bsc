/*
 * SSACT.h
 *
 *  Created on: Aug 6, 2010
 *      Author: aislan
 */

#ifndef SSACT_H_
#define SSACT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mysql.h>

#include "constants.h"
#include "ProfileGenerator.h"
#include "ProfileAnalyses.h"
#include "ClusterGenerator.h"
#include "SystemMaintence.h"

typedef struct {
	int id;
	float genres[NUMBER_OF_GEN];
} profile_t;
#endif /* SSACT_H_ */
