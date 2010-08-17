/*
 * SystemMaintence.h
 *
 *  Created on: Aug 17, 2010
 *      Author: aislan
 */

#ifndef SYSTEMMAINTENCE_H_
#define SYSTEMMAINTENCE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mysql.h>

#include "ProfileGenerator.h"
#include "ClusterGenerator.h"

#define CLUSTER
#define ALPHA 100

void updateProfilesTable(MYSQL *conn);

#endif /* SYSTEMMAINTENCE_H_ */
