/*********************************************
 * OPL 12.6.0.0 Model
 * Author: panyi
 * Creation Date: Feb 3, 2015 at 9:18:30 PM
 * Same problem with VDC-EON1_2. 
 * Change the objective: maximum the number of accepted demands while minimize the total cost for
 *     these accepted demands.
 *     MinMax problem - described in ILP Trickss
 *********************************************/

tuple DC{
   key int v_ID;
   int C;
   int S;
   int P;
   float pC;
   float pS;
   float pP;
   float pn_i;
   float pn_e;
 }

 tuple link{
   int src;
   int des;
   float dist;  
 }
 
 tuple demand{
   key int d_ID;
   int ST;
   int ET;
   int Num; //the total number of VDCs of this demand
   float Bud;
 }
 
 tuple VDC{
   key int d_ID;
   key int vdc_ID;
   int RC;
   int RS;
   int RP; 
   //int loc; //constraint mapping location
   int degree; //to indicate the degree of VDC node in the VCI graph
 }
 
 tuple vlink{
   int d_ID;
   int vsrc;
   int vdest;
   int RB; 
 }
 {DC} V = ...;
 {link} E = ...;
 {demand} D = ...;
 {VDC} VV = ...;
 {vlink} VE = ...;
 
 float C_FO = 0.02;
 float C_OA = 0.0625;
 float Reg10 = 1.9; //10 Gbps regenerator, reach is 2500km
 float BVT10 = 2.5;
 int reach = 2500; //for Reg10
 float bcost = 0.00083; //inter region bandwidth cost #/GB/hour
 int F = 400;
 int M = 1;
 float FSU = 12.5;
// float MF = 400*12.5;
 
 dvar float+ capex[d in D];
 dvar float+ opex[d in D];
 dvar boolean x[d in D][k in VV][v in V]; //VDC k mapped to DC v
// dvar boolean x_t[d in D][k in VV][v in V][t in 1 .. 24]; //VDC k mapped to DC v in time slot t
 dvar boolean y_t[d in D][l in VE][e in E][f in 1 .. F][t in 1 .. 24]; //virtual link (k1,k2) mapped to physical link (v1,v2) for demand d in time slot t, frequency slot f is used  
 dvar boolean y[d in D][l in VE][e in E][f in 1 .. F]; //virtual link (k1,k2) mapped to physical link (v1,v2) for demand d, frequency slot f is used
 dvar float totalcost;
 
 //dexpr float totalcost = sum(d in D) (capex[d] + opex[d]);
 minimize totalcost;
 
 subject to{
 sum(d in D) (capex[d] + opex[d]) <= totalcost;
 
 forall(d in D)
       opex[d] == sum(t in d.ST .. d.ET, k in VV, v in V)(k.RC*v.pC + k.RS*v.pS + k.RP*v.pP)*x[d][k][v]  
     + sum(t in d.ST .. d.ET, l in VE: d.d_ID==l.d_ID) l.RB*bcost;
     
// forall(d in D)//, t in 1 .. 24: t == d.ST)
//   capex[d] == sum(k in VV, v in V: d.d_ID==k.d_ID)(v.pn_i + v.pn_e)*x[d][k][v]*k.degree
//   		+ sum(f in 1 .. F, l in VE, e in E: d.d_ID==l.d_ID) y[d][l][e][f] * ceil(l.RB/M/FSU)* ( e.dist * (C_FO+C_OA) + Reg10* floor(e.dist/reach) )
//   		+ sum(l in VE: d.d_ID==l.d_ID) ceil(l.RB/10)*BVT10*2; 
   
 
 //resource capacity constraints
 forall(v in V, t in 1 .. 24){
  	sum(d in D, k in VV: t >= d.ST && t <= d.ET) k.RC * x[d][k][v] <= v.C;
  	sum(d in D, k in VV: t >= d.ST && t <= d.ET) k.RS * x[d][k][v] <= v.S;
  	sum(d in D, k in VV: t >= d.ST && t <= d.ET) k.RP * x[d][k][v] <= v.P;
  }
  
 //Resource allocation region constraint. No two VDCs of one demand are mapped to one same DC.
    //**** a VDC can only be mapped to one DC
  forall(d in D, k in VV:d.d_ID == k.d_ID) {
  	//sum(v in V) x[d][k][v] == 1 ; 
    sum(v in V) x[d][k][v] >= 1 && sum(v in V) x[d][k][v] <= 1 ;
  } 
    //**** a DC can only has at most a VDC of a demand assigned on itself  
  forall(d in D, v in V) {
      sum(k in VV: k.d_ID==d.d_ID) x[d][k][v] <= 1;
  }  
//  forall(d in D, t in d.ST .. d.ET, k in VV, v in V: d.d_ID==k.d_ID)
//    x[d][k][v] == 1 => x_t[d][k][v][t] == 1;
 
  //Spectrum Continuity Constraint

  forall(d in D, l in VE: l.d_ID==d.d_ID ) {
      forall(v1, v2 in V, k1, k2 in VV: k1.d_ID == d.d_ID && k2.d_ID == d.d_ID && l.vsrc==k1.vdc_ID && l.vdest==k2.vdc_ID  ) {
       (x[d][k1][v1]==1 && x[d][k2][v2]==1) => (
          sum(<v1.v_ID,o,c> in E, f in 1 .. F) y[d][l][<v1.v_ID,o,c> ][f] - x[d][k1][v1] * ceil(l.RB/M/FSU) == 0 && //src outflow equal to # of frequency slots
          sum(<i,v2.v_ID,c> in E, f in 1 .. F) y[d][l][<i,v2.v_ID,c>][f] - x[d][k2][v2] * ceil(l.RB/M/FSU) == 0  //des inflow equal to # of frequency slots
          //sum(<i,v.v_ID,c> in E, f in 1 .. F) y[d][l][<i,v.v_ID,c> ][f] - sum(<v.v_ID,o,c> in E, f in 1 .. F) y[d][l][<v.v_ID,o,c>][f] == 0//intermediate
          ) ;  
          
       forall(<i, v1.v_ID,c> in E, f in 1 .. F)  
         (x[d][k1][v1]==1 && x[d][k2][v2]==1) => y[d][l][<i, v1.v_ID,c>][f] == 0; //src no in flow
       forall(<v2.v_ID,o,c> in E, f in 1 .. F) 
         (x[d][k1][v1]==1 && x[d][k2][v2]==1) => y[d][l][<v2.v_ID,o,c>][f] == 0; //des no out flow
       forall(v in (V diff {v1, v2}))
         //(x[d][k1][v1]==1 && x[d][k2][v2]==1) => sum(<i,v.v_ID,c> in E, f in 1 .. F) y[d][l][<i,v.v_ID,c> ][f] - sum(<v.v_ID,o,c> in E, f in 1 .. F) y[d][l][<v.v_ID,o,c>][f] == 0;
         sum(<i,v.v_ID,c> in E, f in 1 .. F) y[d][l][<i,v.v_ID,c> ][f] - sum(<v.v_ID,o,c> in E, f in 1 .. F) y[d][l][<v.v_ID,o,c>][f] == 0;
       forall(v in (V diff {v1, v2}), f in 1 .. F)
         //(x[d][k1][v1]==1 && x[d][k2][v2]==1) => sum(<i,v.v_ID,c> in E) y[d][l][<i,v.v_ID,c> ][f] - sum(<v.v_ID,o,c> in E) y[d][l][<v.v_ID,o,c>][f] == 0;;
        sum(<i,v.v_ID,c> in E) y[d][l][<i,v.v_ID,c> ][f] - sum(<v.v_ID,o,c> in E) y[d][l][<v.v_ID,o,c>][f] == 0;
      
      }
  }  
    //wrong, iterrupt after a long time
    forall(d in D, f in 1.. F, l in VE, e in E:l.d_ID==d.d_ID)
      forall(t in d.ST .. d.ET)
        //y[d][l][e][f] == 1 => y_t[d][l][e][f][t]==1;
        y[d][l][e][f] - y_t[d][l][e][f][t] == 0;
    
    //Spectrum Capacity on each link
    forall(e in E, t in 1 .. 24)
      sum(d in D, f in 1 .. F, l in VE: l.d_ID==d.d_ID) y_t[d][l][e][f][t] <= F;
    
    //Sub-carrier capacity constraint 
    forall(e in E, f in 1 .. F, t in 1 .. 24) //one frequency of an edge can only be used by one route in time t
      sum(d in D, l in VE: l.d_ID==d.d_ID) y_t[d][l][e][f][t] <= 1;

   //Frequency slot consecutiveness constraint
     //make sure that the empolyed frequency slots are consecutive in the frequency domain.
   forall(d in D, l in VE, e in E, f in 1 .. F-1, t in d.ST .. d.ET: l.d_ID==d.d_ID) 
    ( y_t[d][l][e][f][t] - y_t[d][l][e][f+1][t] -1)* (-maxint) >= sum(f1 in f+2 .. F) y_t[d][l][e][f1][t];    
    
   //forall(d in D, l in VE, e in E, f in 1 .. F, t in d.ST .. d.ET-1)
    //s y_t[d][l][e][f][t] == y_t[d][l][e][f][t+1];
   
 }
 
// execute {
// cplex.tilim = 600;
// var d = new Date();
// writeln("at date", d, "objective = ", totalcost, "x[][][] mapping is ", x[d][k][v], "y[][][][] is: ", y[d][f][l][e]);
//}