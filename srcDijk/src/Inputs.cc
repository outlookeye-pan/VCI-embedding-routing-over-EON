#include <iterator>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "BaseDemand.hpp"
#include "BaseVDC.hpp"
#include "BaseVlink.hpp"
#include "Inputs.hpp"
#include <boost/algorithm/string.hpp>

using namespace std;

Inputs::Inputs(const string& file_name)
{
      _import_from_file(file_name);
}

Inputs::~Inputs(void)
{
     clear();
}

void Inputs::_import_from_file(const string& input_file_name)
{
    const char* file_name = input_file_name.c_str();
    BaseDemand demand;
    BaseVDC vdc;
    BaseVlink vlink;

    ifstream ifs(file_name);
    if(!ifs)
    {
        cerr << "The file" << file_name << "cannot be opened." << endl;
        exit(1);
    }
   //reset the numbers of class
   clear();
 
   ifs >> m_trafficNum; // the numberof input demands
   
   int j;
   string line;
   char index;
   while(std::getline(ifs, line))
   {
        index = line[0];
        switch(index)
        {
        case '#': /*comments*/
                 break;
        case 'd': /*input demand*/{
                 string token = line.substr(2,(line.length()-3));
//cout << token << endl;
                 vector<string> newtoken;
                 boost::split(newtoken, token, boost::is_any_of("\t"));
                 
                 demand.setDemandId(atoi(newtoken.at(0).c_str()));
                 demand.setStartTime(atoi(newtoken.at(1).c_str()));
                 demand.setEndTime(atoi(newtoken.at(2).c_str()));
                 demand.setVDCNum(atoi(newtoken.at(3).c_str()));
                 demand.setBud(atof(newtoken.at(4).c_str()));
                 demand.dFail = 0;
                 m_vtTraffic.push_back(demand);
                 j++;        
            }
                 break;
        case 'v': /*input vdcs*/{
                 string token = line.substr(2,(line.length()-3));
//cout << token <<endl;
                 vector<string> newtoken;
                 boost::split(newtoken, token, boost::is_any_of("\t"));

                 vdc.setdId(atoi(newtoken.at(0).c_str()));                 
                 vdc.setVDCId(atoi(newtoken.at(1).c_str()));
                 vdc.setReqC(atoi(newtoken.at(2).c_str()));
                 vdc.setReqS(atoi(newtoken.at(3).c_str()));
                 vdc.setReqP(atoi(newtoken.at(4).c_str()));
                 vdc.setDegree(atoi(newtoken.at(5).c_str()));
               
                 vdc.selNode = 0;
                 vdc.vdcSucc = 0;
                 m_vtVDC.push_back(vdc);  //record task structure 
            }
                 break;
        case 'e': /* input vlinks */{
                 string token = line.substr(2, (line.length()-3));
                 vector<string> newtoken;
                 boost::split(newtoken, token, boost::is_any_of("\t"));

                 vlink.setdlId(atoi(newtoken.at(0).c_str()));
                 vlink.setVsrc(atoi(newtoken.at(1).c_str()));
                 vlink.setVdes(atoi(newtoken.at(2).c_str()));
                 vlink.setRB(atoi(newtoken.at(3).c_str()));

                 m_vtVlink.push_back(vlink);
            }
                 break;

        default: break;
        }//end switch
   


   }//end while    
 
  ifs.close();
} //end function _import_from_file

void Inputs::clear()
{
     m_trafficNum = 0;
     m_vtTraffic.clear();
     m_vtVDC.clear();
     m_vtVlink.clear();

}
