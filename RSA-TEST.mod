/*********************************************
 * OPL 12.6.0.0 Model
 * Author: panyi
 * Creation Date: Jan 4, 2015 at 4:11:08 PM
 * Target: Only do routing and spectrum assignment for demand. Each demand indicates source, destination node and bandwidth
 *         Minimize cost of RSA.
 *********************************************/
 tuple link{
   int srcNode;
   int desNode;
   float dist;  
 }
 
 tuple demand{ 
   int d_ID;
   int src;
   int dst;
   int ST;
   int ET;
   int RB;
 }
 
 {link} E = ...;
 {demand} D = ...;
 {int} Nodes = ...;
 
 float C_FO = 0.02;
 float C_OA = 0.0625;
 float BVT10 = 2.5;
 float bcost = 0.00083; //inter region bandwidth cost #/GB/hour
 int F = 400;
 int M = 1;
 float FSU = 12.5;
 float MF = 400*12.5;
 
 dvar float cost[d in D];
 dvar boolean x[d in D][e in E]; //routing path for demand d
 dvar boolean y[d in D][e in E][f in 1 .. F]; //1, if frequency slots f is used of link e on the route for demand d (route is: d.src ---- d.dst)
 dvar boolean z[d in D][e in E][f in 1 .. F][t in 1 .. 24]; //1 if frequency slot f is used of link e on the route for demand d at time t
 
 //dexpr float totalcost = sum(d in D) cost[d];
 dvar float totalcost;
 minimize totalcost;
 subject to{
 sum(d in D) cost[d] <= totalcost;
 forall(d in D)
   cost[d] == sum(t in d. ST .. d.ET) (d.RB*bcost +ceil(d.RB/10)*BVT10) + sum(f in 1 .. F, e in E, t in d.ST .. d.ET) z[d][e][f][t] * e.dist * (C_FO+C_OA) * ceil(d.RB/M/FSU); 
  
 //frequency continuity 
 forall(d in D){   
   sum(<d.src, o, c> in E, f in 1 .. F) y[d][<d.src,o,c>][f] - ceil(d.RB/M/FSU) == 0;
   sum(<i, d.dst, c> in E, f in 1 .. F) y[d][<i, d.dst, c>][f] - ceil(d.RB/M/FSU) == 0;
   
 //  forall(<i, d.src, c> in E, f in 1 .. F) 
 //    y[d][<i, d.src, c>][f] == 0;
 //  forall(<d.dst, o, c> in E, f in 1 .. F)
 //    y[d][<d.dst, o, c>][f] == 0;
   forall(j in (Nodes diff{d.src, d.dst})) 
       sum(<i,j,c> in E, f in 1 .. F) y[d][<i,j,c>][f] - sum(<j,o,c> in E, f in 1 .. F) y[d][<j,o,c>][f] == 0;
   forall(j in (Nodes diff{d.src, d.dst}), f in 1 .. F)  
     sum(<i,j,c> in E)y[d][<i,j,c>][f] - sum(<j,o,c> in E)y[d][<j,o,c>][f] == 0;
           
   
 }   
 
 forall(d in D, e in E, f in 1.. F)
   forall(t in d.ST .. d.ET)
     //y[d][e][f] == 1 => z[d][e][f][t] == 1;
     y[d][e][f] - z[d][e][f][t] == 0;
 
 forall(e in E, t in 1 .. 24) //frequency capacity of an edge
   sum(d in D, f in 1 .. F) z[d][e][f][t] <= F;
 
 forall(e in E, f in 1 .. F, t in 1 .. 24) //one frequency at one time slot can only be used by one demand
   sum(d in D) z[d][e][f][t] <= 1; 

 
   //after adding this constraints, oplrun can generate results, but IDE is stopped (for 5 demands).
   //for 1, 2, 3 demands, correct results obtained by IDE
 forall(d in D, e in E, f in 1 .. F-2, t in d.ST .. d.ET) 
    ( z[d][e][f][t] - z[d][e][f+1][t] -1)* (-maxint) >= sum(f1 in f+2 .. F) z[d][e][f1][t]; 
 
 //time domain continuity
 //forall(d in D, e in E, f in 1 .. F, t in d.ST .. d.ET-1)
   //z[d][e][f][t] == z[d][e][f][t+1];
   
 }