/* InputGen.java
 * Generate inputs for the VDCmapping1_1 of  OPL formulations
 * Inputs for demand set D; for each demand d, a VDC set V^d. (as in the .dat file in OPL); the connections (vlinks) between VDCs in each V^d set
 * Demand in format: (int d_ID, int ST, int ET, int Num, double Bud)
 * VDC in format: (int d_ID, int vdc_ID, int RC, int RS, int RP, int degree)
 * vlink in format: (int d_ID, int vsrc, int vdes, int RB)
*/

import java.util.*;
import java.io.*;

public class InputGenVDC {
	public static void main(String[] args) throws IOException {
		System.out.println("Give the number of requests:");
		Scanner in = new Scanner(System.in);
		int R = in.nextInt();
	    FileWriter fwdem = new FileWriter("demand.txt");
		FileWriter fwvdc = new FileWriter("vdc.txt");
		FileWriter fwvlink = new FileWriter("vlink.txt");
		Random rand = new Random();
		//String alphabet = "abcdefg";

		try{

			for(int i = 0; i < R; i++) {
				int ST = rand.nextInt(24)+1;
				int ET = rand.nextInt(24)+1;
				while(ET < ST)
					ET = rand.nextInt(24)+1;
				int Num = rand.nextInt(5)+1;
				double Bud = 200000.0;
				//char subLoc = alphabet.charAt(rand.nextInt(alphabet.length()));
				//int RB = 10*(rand.nextInt(20)+1);
				String oneline = "d("+(i+1) + "\t" + ST + "\t" + ET + "\t" + Num + "\t" + Bud +")" + "\n";
				fwdem.write(oneline);

				//generate VCI graph matrix;
				int[][] graph = graphMatrix(Num);

				//generate contents for VDC file
				for(int j = 0; j < Num; j ++) {
					int RC = rand.nextInt(32)+1;
					int RS = RC + rand.nextInt(160)+1;
					int RP = RC*2;
					int degree = 0;
					for(int k = 0; k < Num; k++) {
						if(graph[j][k] != 0)
							degree++;
					}
					String oneline2 = "v(" + (i+1) + "\t" + (j+1) + "\t" + RC + "\t" + RS + "\t" + RP + "\t" + degree + ")" + "\n";
					fwvdc.write(oneline2);
				}

				//generate contents for vlink file
				for(int j = 0; j < Num; j++) {
					for(int k = 0; k<= j; k++) {
						if(graph[j][k] != 0){
							int RB = graph[j][k];
							String oneline3 = "e(" + (i+1) + "\t" + (k+1) + "\t" + (j+1) + "\t" + RB + ")" + "\n";
							fwvlink.write(oneline3);
						}
					}
				}

			}

		} finally {
				fwdem.close();
				fwvdc.close();
				fwvlink.close();
		}
	}//end main

	public static int[][] graphMatrix(int N){
		int[][] vmatrix = new int[N][N];
		Random rand = new Random();
		for(int i = 0; i < N; i ++) {
			for(int j = 0; j <= i; j++) {
				if(i == j) {
					vmatrix[i][j] = 0;
					vmatrix[j][i] = 0;
				}
				else {
					int band = rand.nextInt(17) * 10; //bandwidth requirement is less than 160 Gbps
					//band = band*25;
					vmatrix[i][j] = band;
					vmatrix[j][i] = band;
				}
            }
		}
		return vmatrix;
	}
}
