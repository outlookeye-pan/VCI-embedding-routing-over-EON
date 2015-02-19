#ifndef CLASS_BaseVDC
#define CLASS_BaseVDC


class BaseVDC
{
public:
    int d_vdc_id, vdc_id, RC, RS, RP, degree;

    int selNode; // the node final selected for current vdc
    bool vdcSucc; //flag to show if this vdc is embedded successfuly

    //For tasks
    int getVDCId() const {return vdc_id;}
    void setVDCId(int id_) {vdc_id = id_;}

    int getdId() const {return d_vdc_id;}
    void setdId(int id_) {d_vdc_id = id_;}

    int getReqC() const {return RC;}
    void setReqC(int Presource_) {RC = Presource_;}

    int getReqS() const {return RS;}
    void setReqS(int Dresource_) {RS = Dresource_;}

    int getReqP() const {return RP;}
    void setReqP(int OTresource_) {RP = OTresource_;}

    int getDegree() const {return degree;}
    void setDegree(int degree_) {degree = degree_;}

}; 

#endif
