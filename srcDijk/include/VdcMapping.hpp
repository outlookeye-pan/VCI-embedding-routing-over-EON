//    VdcMapping.hpp
/// Method to map required VDCs to the physical DCs and to map the virtual links to the EON network (RSA implementation)
/// 
/// Sep. 28, 2014
/// Pan Yi
//. ***********************************************************

#ifndef CLASS_VDCMAPPING
#define CLASS_VDCMAPPING

#include <cstdlib>
#include <cassert>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

#include "GraphElements.hpp"
#include "Graph.hpp"
#include "Inputs.hpp"
#include "BaseDemand.hpp"
#include "BaseVDC.hpp"
#include "BaseVlink.hpp"

using namespace std;

class VdcMapping
 {
public:
	//bool vmf[m_vtTraffic.size()][m_vtVDC.size()] = {{false}}; //flag to record if i-th VDC of demand d has been mapped

	VdcMapping();
	~VdcMapping(void);
	double getRes(vector<vector<int> > avaC, vector<vector<int> > avaS, vector<vector<int> > avaP, vector<vector<vector<bool> > > avaB, vector<BaseDemand> D, vector<BaseVDC> VDC, 
		vector<BaseVlink> VL, vector<BaseVertex> node, vector<vector<double> > OPEX, vector<vector<double> > CAPEX, Graph * my_graph_pt, int failNum);
	
	
	int linksSatisfyOnPath (int startTime, int endTime, int fnum, vector<BaseVertex*>  vertices_on_path, int pathLength, Graph* my_graph, vector<vector<vector<bool> > > tmp_avaB);
};
bool PathsSort(BasePath* left, BasePath* right);

#endif