#ifndef CLASS_BaseVlink
#define CLASS_BaseVlink

class BaseVlink {

public:
	int d_vlink_id, vsrc, vdes, RB;

    void setdlId(int id_) {d_vlink_id = id_;}

    void setVsrc(int src_) {vsrc = src_;}

    void setVdes(int des_) {vdes = des_;}

    void setRB(int RB_) {RB = RB_;}
};

#endif