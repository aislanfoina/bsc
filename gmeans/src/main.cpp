//============================================================================
// Name        : gmeans.cpp
// Author      : Aislan G. Foina
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include "GMEANS.hpp"
#include "Partition.hpp"
#include "Point.hpp"

using namespace std;

int main() {
	int num = 30000;
	int dim = 2;

	vector<const Point*> *Data;
	Partition *DataPartition;

	Data = new vector<const Point*>;
	Point *pt;
	for (int i = 0; i < num; i++) {
		vector<double> pnts;
		for (int j = 0; j < dim; j++) {
			pnts.push_back((rand() / 100000.0f + j));
		}

		pt = new Point(pnts);
		Data->push_back(pt);
	}

	DataPartition = new Partition();

	GMEANS *gm;
	gm = new GMEANS();
	gm->Run(*Data, *DataPartition, true);

	printf("************************\nJob done! Clusters = %d\n************************\n",(int)DataPartition->GetNumberOfClusters());

	return 0;
}
