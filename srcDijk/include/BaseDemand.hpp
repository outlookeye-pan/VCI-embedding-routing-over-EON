#ifndef CLASS_BaseDemand
#define CLASS_BaseDemand


class BaseDemand
{
public:
     int d_id, ST, ET;
     int VDC_num;
     double Budget;     

     bool dFail;
     //For jobs
     int getDemandId() const {return d_id;} 
     void setDemandId(int d_id_) {d_id = d_id_;}

     int getStartTime() const {return ST;}
     void setStartTime(int ST_) {ST= ST_;}

     int getEndTime() const {return ET;}
     void setEndTime(int ET_) {ET = ET_;}

     double getBud() const {return Budget;}
     void setBud(double Budget_) {Budget = Budget_;}

     int getVDCNum() const {return VDC_num;}
     void setVDCNum(int num_) {VDC_num = num_;}
     
};



#endif
