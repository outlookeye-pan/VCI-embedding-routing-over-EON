// main.cc                                                                     
// The main function for VCI embedding & RSA problem 
//                                                                     
// Time: Sep. 28, 2014
// 
// Author: Pan Yi     
 // * We assume each fiber link has 400 frequency slots, from 1 to 400 ; The frequency width of each frequency slot unit is 12.5 GHz.
 // * We assume that nodes 2,3, 4, 5 are in central region. Ccost, Scost, Pcost: 0.0018  0.00038 0.00042
 //                        1 is in west region,                    : 0.0018  0.00045 0.00055
 //                        6 is in east region.                    : 0.0020  0.00041 0.00056
 // * IP node cost (c.u.), capacity 160 Gbps: 9
 // * EON node cost (c.u.) is: 1.5*WDMnode = [1.5*(N*(WSS + 0.8+0.8) + N*2*0.5+N*4*0.9)]*0.5 = 33.40*N  // times 0.5 means node for 40 channels, N node degree
 //    WSS cost is 38.33 c.u. (estimate value from the results in CAPEX study of IP-over-EON paper)
 // * 10 Gbps BVT cost(c.u.) : 2.5; reach 2500 km
 //       400 Gbps BVT is: 65.625; reach 400 km
 // * C_FO, cost per km per GHz of using the already deployed optical fiber: 0.02 (including regenerator cost)
 // * C_OA, optical amplifier cost, in general reach is 80 km: 5; so per km cost is 5/80=0.0625 

 // * reach distance (km) for modulation formats.
 //     BPSK: 2500   QPSK: 1500    8-QAM: 750     16-QAM: 375

 // CAPEX should not related with time; rental OPEX should related with time duration

//.******************************************
#include <cstdlib>
#include <cassert>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <time.h>

#include "Graph.hpp"  
#include "GraphElements.hpp"
#include "Inputs.hpp"
#include "BaseDemand.hpp"
#include "BaseVDC.hpp"
#include "BaseVlink.hpp"
#include "VdcMapping.hpp"


using namespace std;

//sort the job according to job duration
bool SJFsort(BaseDemand left, BaseDemand right)
{
  return left.ET-left.ST+1 < right.ET-right.ST+1;
}

//random job sorting, generate ramdom 'gen'
ptrdiff_t mygen(ptrdiff_t i) 
{
  return rand()%i;
}
  //pointer object to it:
ptrdiff_t (*p_mygen)(ptrdiff_t) = mygen;

//sort the job according to job start time
bool ESJFsort(BaseDemand left, BaseDemand right)
{
  return left.ST < right.ST; 
}

//sort the job according to job structure, simple job structure first
bool SSFsort(BaseDemand left, BaseDemand right)
{  
  return left.VDC_num < right.VDC_num;
}

//sort node in topology
bool sortNode(BaseVertex left, BaseVertex right) {
  return (left.Ccost*0.5 + left.Scost*0.25 + left.Pcost*0.25) < (right.Ccost*0.5 + right.Scost*0.25 + right.Pcost*0.25);
}


///////    main function       ////////
///////                        ////////
///////////////////////////////////////
int main(int argc, char *argv[])
{
  //record code start time
  clock_t start_time = clock();

  /**** read in three input files****/
  Graph *  my_graph = new Graph("../data/nsfnet-topo.dat");
  cout << "**** Graph read in done ****" <<endl;
  Inputs my_inputs("../data/1800input-5.dat"); //inputs.dat include demand, vdc, vlink
  cout << "**** Demands read in done ****" <<endl;

  //.**** get current available resources on each node in each timeslot  ****.//
  ///////define three vectors to store available resources on each node/link in each timeslot, so 2-dimentional
  vector<vector<int> > avaC(my_graph->m_nVertexNum, vector<int> (Smax));
  vector<vector<int> > avaS(my_graph->m_nVertexNum, vector<int> (Smax));
  vector<vector<int> > avaP(my_graph->m_nVertexNum, vector<int> (Smax));
  
  vector<BaseVertex>::iterator iter1 = my_graph->m_vtNodes.begin(), iter1_end = my_graph->m_vtNodes.end();
     /////available resources on node
  for(int i=0; i<my_graph->m_nVertexNum; i++,iter1++)
  {   
    for(int j=0; j<Smax; j++)
    {    
      avaC[i][j] = iter1->C; // - iter2->busyPNums;
      avaS[i][j] = iter1->S; // - iter2->busyDNums;
      avaP[i][j] = iter1->P; // - iter2->busyOTNums;
//      cout << "In noded "<< i << ", in time slot " << j << ", available cpu is " << avaC[i][j] << ", availabe storage is " << avaS[i][j]
//      << ", available port is " << avaP[i][j] << endl;
    }
  }

    vector<vector<double> > OPEX(my_inputs.m_vtTraffic.size(), vector<double> (5,0)); //for VDCs of demands, opex cost, related with time duration
    vector<vector<double> > CAPEX(my_inputs.m_vtTraffic.size(), vector<double> (5,0)); //for VDCs of demands, capex cost
//    cout << "cost size one " << cost.size() << ", cost 2nd dimension size: "<< cost[0].size() << endl;
    double totalCosts = 0;  
    int failNum = 0;
    VdcMapping ini_sol;

    iter1 = my_graph->m_vtNodes.begin(); // set iter1 back to begin, and then sort nodes in topology
    sort(iter1, iter1_end, sortNode); 
      switch (atoi(argv[1]))
      {
        case 0:
        {
            totalCosts = ini_sol.getRes(avaC, avaS, avaP, my_graph->avaB, my_inputs.m_vtTraffic, my_inputs.m_vtVDC, my_inputs.m_vtVlink, 
              my_graph->m_vtNodes, OPEX, CAPEX, my_graph, failNum);
            cout<< "Total cost for all jobs are: "<< totalCosts << endl;
            //cout<< "The failed demand number is " << failNum <<endl;
            clock_t end_time = clock();
            cout<< "Running time is: " << static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;

        }
        break;
  //     case 1:
  //     {
  //       sort(my_inputs.m_vtTraffic.begin(), my_inputs.m_vtTraffic.end(), SJFsort);
  //       totalCosts = ini_sol.getRes(avaP, avaD, avaOT, avaB, my_inputs.m_vtTraffic, my_inputs.m_vtTask, my_graph.m_nVertexNum,cost, my_graph.m_vtNodes, my_graph.m_vtLinks, my_graph.m_vtRoutes);
  //       cout<< "Total cost for all jobs are: "<< totalCosts << endl;
  //       clock_t end_time = clock();
  //       cout<< "Running time is: " << static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
  //     }
  //     break;
  //     case 2:
  //     {
  //       random_shuffle(my_inputs.m_vtTraffic.begin(), my_inputs.m_vtTraffic.end(), p_mygen);
  //       totalCosts = ini_sol.getRes(avaP, avaD, avaOT, avaB, my_inputs.m_vtTraffic, my_inputs.m_vtTask, my_graph.m_nVertexNum,cost, my_graph.m_vtNodes, my_graph.m_vtLinks, my_graph.m_vtRoutes);
  //       cout<< "Total cost for all jobs are: "<< totalCosts << endl;
  //       clock_t end_time = clock();
  //       cout<< "Running time is: " << static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
  //     }
  //     break;
  //     case 3:
  //     {
  //       sort(my_inputs.m_vtTraffic.begin(), my_inputs.m_vtTraffic.end(), ESJFsort);
  //       totalCosts = ini_sol.getRes(avaP, avaD, avaOT, avaB, my_inputs.m_vtTraffic, my_inputs.m_vtTask, my_graph.m_nVertexNum,cost, my_graph.m_vtNodes, my_graph.m_vtLinks, my_graph.m_vtRoutes);
  //       cout<< "Total cost for all jobs are: "<< totalCosts << endl;
  //       clock_t end_time = clock();
  //       cout<< "Running time is: " << static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
  //     }
  //     break;
  //     case 4:
  //     {
  //       sort(my_inputs.m_vtTraffic.begin(), my_inputs.m_vtTraffic.end(), SSFsort);
  //       totalCosts = ini_sol.getRes(avaP, avaD, avaOT, avaB, my_inputs.m_vtTraffic, my_inputs.m_vtTask, my_graph.m_nVertexNum,cost, my_graph.m_vtNodes, my_graph.m_vtLinks, my_graph.m_vtRoutes);
  //       cout<< "Total cost for all jobs are: "<< totalCosts << endl;
  //       clock_t end_time = clock();
  //       cout<< "Running time is: " << static_cast<double>(end_time-start_time)/CLOCKS_PER_SEC*1000<<"ms"<<endl;
  //     }
  //     break;

       default: break;
     }
    



}//end main


