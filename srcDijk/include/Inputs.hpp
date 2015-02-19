#ifndef CLASS_Inputs
#define CLASS_Inputs

#include <vector>
#include "BaseDemand.hpp"
#include "BaseVDC.hpp"
#include "BaseVlink.hpp"
#include "GraphElements.hpp"

using namespace std;

class Inputs : public BaseDemand, public BaseVDC, public BaseVlink
{
public:
       //vector structure to store the demand, VDC, vlink
       vector<BaseDemand> m_vtTraffic; //this is for demand
       vector<BaseVDC> m_vtVDC; //this is for VDC
       vector<BaseVlink> m_vtVlink; //for virtual link between VDCs
       
       int m_trafficNum; //demand number in the inputs file
       int m_vdcNum;

public:
       Inputs(const string& file_name);
       ~Inputs(void);
   
       void clear();

       BaseDemand get_traffic(int i)
       {
           return m_vtTraffic.at(i);

       }
  
       int get_number()
       {
           return m_trafficNum;
       }

private:
       void _import_from_file(const std::string& file_name);

}; // end class Inputs




#endif
