////////////////////////////////////
/// VdcMapping.cc
/// For each demand, Map the VDC requirements to physical DCs. Complete the RSA for the spectrum requirements  between VDCs
/// Sep. 30, 2014
/// Pan Yi
///
///////////////////////////////////
#include <iterator>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <map>
#include <algorithm>
#include <cmath>

#include "GraphElements.hpp"
#include "Graph.hpp"
#include "BaseDemand.hpp"
#include "BaseVDC.hpp"
#include "BaseVlink.hpp"
#include "Inputs.hpp"
#include "VdcMapping.hpp"
#include <boost/algorithm/string.hpp>
#include "DijkstraShortestPathAlg.hpp"

using namespace std;
   //BPSK
//#define M 1 //BPSK, M=2 QPSK, M=3 8-QAM, M=4 16-QAM
//#define reach 2000 //for BPSK
   // QPSK
#define M 2
#define reach 1500 //QPSK
   //8-QAM
//#define M 3
//#define reach 750
   //16-QAM
//#define M 4
//#define reach 375

#define C_FO 0.02 //cost per km per GHz of using the already deployed optical fiber: 0.02 (including regenerator cost)
#define C_OA 0.0625 //optical amplifier cost, in general reach is 80 km: 5; so per km cost is 5/80=0.0625
#define BVT10 2.5 //reach 2500 km
//#define BVT400 62.625 //reach 400km
#define Reg10 1.9 //10 Gbps regenarator, reach 2500 km
//#define Reg400 27.2 //400 Gbps regenerator, reach 400 km
#define bcost 0.00083


VdcMapping::VdcMapping() {}

VdcMapping::~VdcMapping(void) {}

//sort the shortest paths according to weight
bool PathsSort(BasePath* left, BasePath* right)
{
  return (left->Weight() < right->Weight());
}

int VdcMapping::linksSatisfyOnPath (int startTime, int endTime, int fnum, vector<BaseVertex*>  vertices_on_path, int pathLength, Graph* my_graph, 
	vector<vector<vector<bool> > > tmp_avaB) {
	int fStartIndex = 0;
	vector<BaseVertex*>::iterator iter1 = vertices_on_path.begin(), iter1_end = vertices_on_path.end();
//	BaseVertex sourceVertex =  * vertices_on_path.begin();
	int f = 0, lcount = 0, t = startTime, tcount = 0, fcount = 0;
	bool link_done = false;
	while(iter1 != iter1_end-1) {
			int edgeCode = my_graph->get_edge_code(*iter1, *(iter1+1));
			t = startTime;
			if(lcount == 0) { //deal with the first link on a path
				tcount = 0;
				while(t<=endTime) {
					fcount = 0;//frequency slot count, to record how many f slots satisfy for each time
					while(f < F) {
						if(tmp_avaB[edgeCode][t-1][f] == true) {
							fcount ++;
							if(fcount == fnum) {
								fStartIndex = f+1-fnum;
								f = fStartIndex;
								tcount++;
								if(tcount == endTime-startTime+1) {
									tcount = 0;
									lcount += 1; //the first link succeed
									link_done = true;
									break;
								}
								else {
									t ++;
									break; //go to next time slot
								}
							}
							else {
								f++;
							}
						}
						else { //avaB not satisfy
							if(t != startTime) {
								f++;
								t = startTime;
								fcount = 0;
								tcount = 0;
								break;
							}
							else {
								f++;
								fcount = 0;

							}
						}
					}
					if(link_done == true) {
						//link_done = false;
						iter1 ++;
						break; //the first link done
					}
					else {
						if(f >= F)
							break;
					}

				}
			} //first link end
			else {
				t = startTime;
				f = fStartIndex;
				tcount = 0;
				while(t <= endTime) {
					fcount = 0;
					while(f < F) {
						if(tmp_avaB[edgeCode][t-1][f] == true) {
							fcount ++;
							if(fcount == fnum) {
								tcount ++;
								fcount = 0;
								f = fStartIndex;
								if(tcount == endTime-startTime+1) {
									tcount = 0;
									lcount += 1;
									link_done = true;
									break;
								}
								else {
									t++;
									break; //go to next time slot
								}
							}
							else {
								f++;
							}

						}
						else {
							//start over, back to the first link of current path
							f = f+1;
							lcount = 0;
							t = startTime;
							iter1 = vertices_on_path.begin();
							break;
						}
					}
					if(link_done == true) {
						//link_done = false;
						iter1 ++;
						break; //go to next link on path
					}
					else {
						if(f < F) {
							if(lcount == 0) //current link fail, choose a new start f, go back check first link
								break;
							else //current slot of current link done, go to next time slot
								continue;
						}
						else
							break;
					}
				}
			} //other links on a path
			if(link_done == true) {
				link_done = false;
			}
			else
				break;
	}//iter1
	if(lcount == pathLength)
		return fStartIndex;
	else
		return -1;

	//return fStartIndex;
}

double VdcMapping::getRes(vector<vector<int> > avaC, vector<vector<int> > avaS, vector<vector<int> > avaP, vector<vector<vector<bool> > > avaB, vector<BaseDemand> D, vector<BaseVDC> VDC, 
		vector<BaseVlink> VL, vector<BaseVertex> node, vector<vector<double> > OPEX, vector<vector<double> > CAPEX, Graph * my_graph_pt, int failNum) {
	//
	vector<BaseDemand>::iterator iter_demand = D.begin(), iter_demand_end = D.end();
	vector<BaseVDC>::iterator iter_vdc = VDC.begin(), iter_vdc_end = VDC.end();
	vector<BaseVlink>::iterator iter_vlink = VL.begin(), iter_vlink_end = VL.end();
	vector<BaseVertex>::iterator iter_node = node.begin(), iter_node_end = node.end();
	//vector<BaseVDC>::iterator iter_release;

	int timeCount =0; //set for count if a node satisfy task during the task duration
	map<int, bool> usedNode; //record current visiting node is used or not 
	double tempcost = 0;
	bool vmf[D.size()][5];
	memset(vmf, false, sizeof(vmf)); //since the vmf array is variable-sized array, so cannot be initialized like: bool vmf[D.size()][5] == {{false}}
	cout << "test into VdcMapping\n";
	bool curfail = false; //mark current demand fail


	vector<vector<int> > tmp_avaC(avaC.size(), vector<int> (Smax));
	vector<vector<int> > tmp_avaS(avaS.size(), vector<int> (Smax));
	vector<vector<int> > tmp_avaP(avaP.size(), vector<int> (Smax));
	//cout << "my_graph_pt->avaB.size() is " << my_graph_pt->avaB.size() << endl;
	vector<vector<vector<bool> > > tmp_avaB(avaB.size(), vector<vector<bool> > (Smax, vector<bool> (F)));
	//use tmp_avalable_resource during the processing
	for(int i = 0; i < avaC.size(); i++) {
		for(int j = 0; j < Smax; j++) {
			tmp_avaC[i][j] = avaC[i][j];
			tmp_avaS[i][j] = avaS[i][j];
			tmp_avaP[i][j] = avaP[i][j];
		}
	}
	for(int i = 0; i < avaB.size(); i++) {
		for(int j = 0; j < Smax; j++) {
			for(int f = 0; f < F; f++) {
				tmp_avaB[i][j][f] = avaB[i][j][f];
				//cout << "On link " << i << " in time slot " << j << " at frequency " << f << ", the frequency available is: " << tmp_avaB[i][j][f] << endl;
			}
		}
	}

	//mapping method begin
	for(int d = 0; d < D.size(); d++, iter_demand++) {
		iter_vdc = VDC.begin();
		usedNode.clear();
		iter_node = node.begin();
		for(; iter_vdc != iter_vdc_end; iter_vdc++) {
			if(iter_vdc->d_vdc_id == iter_demand->d_id) {
				if(vmf[d][iter_vdc->vdc_id -1] == 0) { //current VDC is not mapped yet
					for(iter_node=node.begin(); iter_node!=iter_node_end; iter_node++) {
						//cout<< "Used node map size is " << usedNode.size() <<endl;
						if(usedNode.find(iter_node->nodeID) != usedNode.end()) //node is used
							continue;
						else {
							for(int t = iter_demand->ST; t <= iter_demand->ET; t++) {
								if(tmp_avaC[iter_node->nodeID][t-1] >= iter_vdc->RC && tmp_avaS[iter_node->nodeID][t-1] >= iter_vdc->RS && tmp_avaP[iter_node->nodeID][t-1] >= iter_vdc->RP){
									timeCount ++;
								}
								else
									break;
							}
							if(timeCount == (iter_demand->ET - iter_demand->ST + 1) ) {
								usedNode.insert(pair<int, bool> (iter_node->nodeID, true)); //mark this node is used by current demand
								//cout << "node used map size is " << usedNode.size()<<endl;
								vmf[d][iter_vdc->vdc_id -1] = true; //mark current VDC is mapped
								iter_vdc->selNode = iter_node->nodeID; //mark mapped node for current VDC
								//opex
								tempcost = (iter_demand->ET - iter_demand->ST + 1)*(iter_node->Ccost*iter_vdc->RC + iter_node->Scost*iter_vdc->RS + iter_node->Pcost*iter_vdc->RP);
								OPEX[d][iter_vdc->vdc_id -1] += tempcost;
								//capex
								//tempcost = iter_node->ipNodeCost + iter_node->eonNodeCost;
								//CAPEX[d][iter_vdc->vdc_id-1] += tempcost;
//								cout << "For vdc " << iter_vdc->vdc_id-1 << " of demand " << d << ", the opex is " << OPEX[d][iter_vdc->vdc_id-1] <<endl;
								for(int t = iter_demand->ST; t <= iter_demand->ET; t++) {
									tmp_avaC[iter_node->nodeID][t-1] -= iter_vdc->RC;
									tmp_avaS[iter_node->nodeID][t-1] -= iter_vdc->RS;
									tmp_avaP[iter_node->nodeID][t-1] -= iter_vdc->RP;
								}
								timeCount = 0;
								break;
							}
							else
								timeCount = 0; //go to check next node

						}
					}
				}
//				else { //current VDC is mapped already
//					continue;
//				}
				//current VDC is mapped already or just complete mapping, then check its connected vlinks
				if(iter_vdc->vdc_id == iter_demand->VDC_num) { //current VDC is the last VDC in current demand
					iter_node = node.begin();
					break;
				}
				else { //not last VDC, find adjacent VDCs with connected vlinks
					//find shortest paths from the DC that the current VDC mapped to every other nodes in topology
					vector<BaseVertex>::iterator iter_n2 = node.begin();
					vector<BasePath*> SP;
					DijkstraShortestPathAlg shortest_path_alg(my_graph_pt);
					for(; iter_n2 != node.end(); iter_n2++) {
						if(iter_n2->nodeID != iter_node->nodeID) {
							BasePath* result = shortest_path_alg.get_shortest_path(my_graph_pt->get_vertex(iter_node->nodeID), my_graph_pt->get_vertex(iter_n2->nodeID));
//							result->PrintOut(cout);
							SP.push_back(result);
						}
						//...........................
					}
					vector<BasePath*>::iterator iter_sp = SP.begin(), iter_sp_end = SP.end();
					sort(SP.begin(), SP.end(), PathsSort); //sort the paths in SP
					
					iter_vlink = VL.begin();
					for(; iter_vlink!= VL.end(); iter_vlink++) {
						if(iter_vlink->d_vlink_id == iter_demand->d_id && iter_vlink->vsrc == iter_vdc->vdc_id) {
							//find the iter_vdc->vdes VDC
							vector<BaseVDC>::iterator iter_vdc_vdes = VDC.begin(), iter_vdc_vdes_end = VDC.end();
							for(; iter_vdc_vdes != iter_vdc_vdes_end; iter_vdc_vdes++) {
								if(iter_vdc_vdes -> d_vdc_id == iter_demand->d_id && iter_vdc_vdes->vdc_id == iter_vlink->vdes)
									break;
							}
							//cout << "here?????\n";
							if(vmf[d][iter_vdc_vdes->vdc_id-1] == false) { //if current vlink destination vdc is not mapped yet. map it first, then check link on path
								for(iter_sp = SP.begin(); iter_sp != iter_sp_end; iter_sp++) {
									//check if des(sp) has enough resources to map vlink->des
									   //note: length() function return the number of nodes in the path, not number of links of the path
									int desIndex = (*iter_sp)->length()-1;
									BaseVertex* des = (*iter_sp)->GetVertex(desIndex); //map to the destination node of shortest path
									/**** find iter_node_tmp that has same nodeID with iterator des
									Since ierator des does not have resource and unit cost information on each node, only has node id
									*****/
									vector<BaseVertex>::iterator iter_node_tmp = node.begin(), iter_node_tmp_end = node.end();
									for(; iter_node_tmp != iter_node_tmp_end; iter_node_tmp++){
										if(iter_node_tmp->nodeID == des->nodeID)
											break;
									}
									int count = 0;
									if(usedNode.find(des->nodeID) == usedNode.end()) {
										for(int t = iter_demand->ST; t <= iter_demand->ET; t++) {
											if(tmp_avaC[des->nodeID][t-1] >= iter_vdc_vdes->RC && tmp_avaS[des->nodeID][t-1] >= iter_vdc_vdes->RS && tmp_avaP[des->nodeID][t-1] >= iter_vdc_vdes->RP)
												count++;
											else { //not enough resource in current timeslot on the des node of current path
												count = 0;
												break; //go check another path
											}
										}
									
										if(count == iter_demand->ET - iter_demand->ST +1) {
											// node resource satisfied, then consider spectrums of links on path
											int fNum = ceil(iter_vlink->RB/M/12.5);
											vector<BaseVertex*> verticesOnPath;
											for(int i = 0; i < (*iter_sp)->length(); i++)
												verticesOnPath.push_back((*iter_sp)->GetVertex(i));
											int fStartIndex = linksSatisfyOnPath(iter_demand->ST, iter_demand->ET, fNum, verticesOnPath, (*iter_sp)->length()-1, my_graph_pt, tmp_avaB);
//											cout << "here????, fStartIndex" << fStartIndex << endl;
											if( fStartIndex != -1) {
												//update frequency resource for each time of this path
												//update mapped destination node resource 
												//update cost
												usedNode.insert(pair<int, bool> (des->nodeID, true)); //mark this node is used by current demand
//												cout << "Map node when deal with connections.  **** used node map size is " << usedNode.size() << endl;
												vmf[d][iter_vdc_vdes->vdc_id-1] = true;
												iter_vdc_vdes->selNode = iter_node_tmp->nodeID; //mark mapped node
												//temp cost for node
												//opex
												tempcost = (iter_demand->ET - iter_demand->ST + 1)*(iter_node_tmp->Ccost*iter_vdc_vdes->RC + iter_node_tmp->Scost*iter_vdc_vdes->RS 
													+ iter_node_tmp->Pcost*iter_vdc_vdes->RP + iter_vlink->RB*bcost);
												OPEX[d][iter_vdc_vdes->vdc_id -1] += tempcost;
												//capex
												   //link source map node (IP, EON ) and destination map node (IP, EON), and transponder pair
												tempcost = (iter_node->ipNodeCost + iter_node->eonNodeCost) + (iter_node_tmp->ipNodeCost + iter_node_tmp->eonNodeCost) + ceil(iter_vlink->RB/10)*BVT10*2; //
												CAPEX[d][iter_vdc_vdes->vdc_id-1] += tempcost;
												//capex cost for path
												tempcost = (*iter_sp)->Weight() * (fNum * C_FO + C_OA) + floor((*iter_sp)->Weight()/reach)*Reg10*ceil(iter_vlink->RB/10);
												CAPEX[d][iter_vdc_vdes->vdc_id-1] += tempcost;
												vector<BaseVertex*>::iterator iter1 = verticesOnPath.begin(), iter1_end = verticesOnPath.end();
//												cout << "For link destination vdc " << iter_vdc_vdes->vdc_id-1 << " of demand " << d << ", the opex is " << OPEX[d][iter_vdc_vdes->vdc_id -1]
//													<< ", the capex is " << CAPEX[d][iter_vdc_vdes->vdc_id-1] <<endl;
												for(int t = iter_demand->ST; t <= iter_demand->ET; t++) {
													//node
													tmp_avaC[des->nodeID][t-1] -= iter_vdc_vdes->RC;
													tmp_avaS[des->nodeID][t-1] -= iter_vdc_vdes->RS;
													tmp_avaP[des->nodeID][t-1] -= iter_vdc_vdes->RP;
													//link
													for(iter1 = verticesOnPath.begin(); iter1 != iter1_end-1; iter1++) {
														int edgeCode = my_graph_pt->get_edge_code(*iter1, *(iter1+1));
														for(int f = fStartIndex; f < fStartIndex+fNum; f++) {
															tmp_avaB[edgeCode][t-1][f] = false;
														}
													}
												}
												break; //go to next adjacent VDC
											}
											//else ///iter_sp go to next possible shortest path
										}
									}
									//else destination node of current path was used for current demand, go to next path directly
								}
							}
							else {//the current vlink destination vdc is already mapped. directly find path for current vlink
								//..../
								//....
								BasePath* result = shortest_path_alg.get_shortest_path(my_graph_pt->get_vertex(iter_node->nodeID), my_graph_pt->get_vertex(iter_vdc_vdes->selNode));
								int fNum = ceil(iter_vlink->RB/M/12.5);
								vector<BaseVertex*> verticesOnPath;
								for(int i=0; i < result->length(); i++)
									verticesOnPath.push_back(result->GetVertex(i));
								int fStartIndex = linksSatisfyOnPath(iter_demand->ST, iter_demand->ET, fNum, verticesOnPath, result->length()-1, my_graph_pt, tmp_avaB);
								
								if( fStartIndex != -1) {
									 //opex for bandwidth
									tempcost = (iter_demand->ET - iter_demand->ST + 1)*( iter_vlink->RB*bcost);
									OPEX[d][iter_vdc_vdes->vdc_id -1] += tempcost;
									//capex for path
									tempcost = result->Weight() * (fNum * C_FO + C_OA) + floor(result->Weight()/reach)*Reg10*ceil(iter_vlink->RB/10);
									CAPEX[d][iter_vdc_vdes->vdc_id-1] += tempcost;
									vector<BaseVertex*>::iterator iter1 = verticesOnPath.begin(), iter1_end = verticesOnPath.end();
//									cout << "Adjacent VDC is mapped already. For link destination vdc " << iter_vdc_vdes->vdc_id-1 << " of demand " << d << ", the opex is " << OPEX[d][iter_vdc_vdes->vdc_id -1]
//													<< ", the capex is " << CAPEX[d][iter_vdc_vdes->vdc_id-1] <<endl;
									for(int t = iter_demand->ST; t <= iter_demand->ET; t++) {
										//link
										for(iter1 = verticesOnPath.begin(); iter1 != iter1_end-1; iter1++) {
											int edgeCode = my_graph_pt->get_edge_code(*iter1, *(iter1+1));
											for(int f = fStartIndex; f < fStartIndex+fNum; f++) {
												tmp_avaB[edgeCode][t-1][f] = false;
											}
										}
									}
								}
								else {//clear the cost of current demand, drop demand
									//go to branch "if(vmf[d][iter_vdc_vdes->vdc_id-1] == false){" in line 394
									vmf[d][iter_vdc_vdes->vdc_id-1] == false;
								}

							}
							//after all iter_sp loop, if current iter_vdc_vdes is not mapped, then need to start over for current demand
							// or simply process. Just mark current demand failed, clear the cost for current demand. go to next demand
							if(vmf[d][iter_vdc_vdes->vdc_id-1] == false){
								//cout << "goes here???\n";
								failNum ++;
								curfail = true;
								//clear cost
								vector<BaseVDC>::iterator iter_vdc_release = VDC.begin();
								for(; iter_vdc_release != VDC.end(); iter_vdc_release++){
									if(iter_vdc_release->d_vdc_id == iter_demand->d_id) {
										CAPEX[iter_demand->d_id-1][iter_vdc_release->vdc_id-1] = 0;
										OPEX[iter_demand->d_id-1][iter_vdc_release->vdc_id-1] = 0;
									}
								}
								break; // go to line 366, final goal is go to next demand
							}
						}
					}//for l--VL
					if(curfail == true) {
						curfail = false;
						break; //go to next demand
					}
				} //not last vdc in demand
			} //if vdc is in current demand
		} //vdc for loop
	} //demand for loop

	//sum total cost for all demands
	double totalcost = 0;
	double totalcapex=0, totalopex=0;
	iter_vdc = VDC.begin();
	iter_demand = D.begin();
	for(; iter_demand != iter_demand_end; iter_demand++) {
		iter_vdc = VDC.begin();
		for(; iter_vdc!= iter_vdc_end; iter_vdc++) {
			if(iter_demand->d_id == iter_vdc->d_vdc_id) {
				totalcapex += CAPEX[iter_demand->d_id-1][iter_vdc->vdc_id-1];
				totalopex += OPEX[iter_demand->d_id-1][iter_vdc->vdc_id-1];
				cout << "For demand " << iter_demand->d_id << ", vdc " << iter_vdc->vdc_id << " the CAPEX and OPEX are " << CAPEX[iter_demand->d_id-1][iter_vdc->vdc_id-1] << " VS. "
				<< OPEX[iter_demand->d_id-1][iter_vdc->vdc_id-1] << " *-*-*-*-*" << endl;
			}
		}

	}
	cout<< "Total CAPEX is " << totalcapex << ", and total OPEX is " << totalopex <<endl;
	cout << "The failed demand number is " << failNum << endl;
	totalcost = totalcapex + totalopex;
	return totalcost;
} //getRes

