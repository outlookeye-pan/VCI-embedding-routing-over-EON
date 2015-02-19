///////////////////////////////////////////////////////////////////////////////
///  GraphElements.h
///  <TODO: insert file description here>
///
///  Modification based on the Online Dijkstra algorithm. Add self defined class and data structure
///  Sep. 18, 2014
///
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <iostream>

template<class T>
class WeightGreater
{
public:
	// Determine priority.
	bool operator()(const T& a, const T& b) const
	{
		return a.Weight() > b.Weight();
	}

	bool operator()(const T* a, const T* b) const
	{
		return a->Weight() > b->Weight();
	}
};

template<class T>
class WeightLess
{
public:
	// Determine priority.
	bool operator()(const T& a, const T& b) const
	{
		return a.Weight() < b.Weight();
	}

	bool operator()(const T* a, const T* b) const
	{
		return a->Weight() < b->Weight();
	}
};

//////////////////////////////////////////////////////////////////////////
// A class for the object deletion
//////////////////////////////////////////////////////////////////////////
template<class T>
class DeleteFunc
{
public:
	void operator()(const T* it) const
	{
		delete it;
	}
};



/**************************************************************************
*  BaseVertex
*  <TODO: insert class description here>
*
*
*  @remarks <TODO: insert remarks here>
*
*  @author Yan Qi @date 6/6/2010
**************************************************************************/
class BaseVertex
{
	double m_dWeight;
public:
	int nodeID;
	int C;
    int S;
    int P;
    double Ccost;
    double Scost;
    double Pcost;
    //add elements for multi-layer
    double ipNodeCost;
    double eonNodeCost;

	double Weight() const { return m_dWeight; }
	void Weight(double val) { m_dWeight = val; }

	int getID() const {return nodeID;}
    void setID(int ID_) {nodeID = ID_;}

    int getCn() const {return C;}
    void setCn(int C_) {C = C_;}

    int getSn() const {return S;}
    void setSn(int S_) {S = S_;}

    int getPn() const {return P;}
    void setPn(int P_) {P = P_;}

    double getCPUCost() const {return Ccost;}
    void setCPUCost(double Ccost_) {Ccost = Ccost_;}

    double getStorageCost() const {return Scost;}
    void setStorageCost(double Scost_) {Scost = Scost_;}

    double getPortCost() const {return Pcost;}
    void setPortCost(double Pcost_) {Pcost = Pcost_;}

    double getIPNode() const{return ipNodeCost;}
    void setIPNode(double ipNodeCost_) {ipNodeCost = ipNodeCost_;}

    double getEONNode() const{return eonNodeCost;}
    void setEONNode(double eonNodeCost_) {eonNodeCost = eonNodeCost_;}

	void PrintOut(std::ostream& out_stream)
	{
		out_stream << nodeID;
	}
};


/**************************************************************************
*  BasePath
*  <TODO: insert class description here>
*
*
*  @remarks <TODO: insert remarks here>
*
*  @author Yan Qi @date 6/6/2010
**************************************************************************/
class BasePath
{
protected:

	int m_nLength;
	double m_dWeight;
	std::vector<BaseVertex*> m_vtVertexList;

public:
	BasePath(const std::vector<BaseVertex*>& vertex_list, double weight)
		:m_dWeight(weight)
	{
		m_vtVertexList.assign(vertex_list.begin(), vertex_list.end());
		m_nLength = m_vtVertexList.size();
	}
	~BasePath(void){}

	double Weight() const { return m_dWeight; }
	void Weight(double val) { m_dWeight = val; }

	int length() { return m_nLength; }


	BaseVertex* GetVertex(int i)
	{
		return m_vtVertexList.at(i);
	}

	bool SubPath(std::vector<BaseVertex*>& sub_path, BaseVertex* ending_vertex_pt)
	{

		for (std::vector<BaseVertex*>::const_iterator pos = m_vtVertexList.begin();
			pos != m_vtVertexList.end(); ++pos)
		{
			if (*pos != ending_vertex_pt)
			{
				sub_path.push_back(*pos);
			}else
			{
				//break;
				return true;
			}
		}

		return false;
	}

	// display the content
	void PrintOut(std::ostream& out_stream) const
	{
		out_stream << "Distance: " << m_dWeight << " Length: " << m_vtVertexList.size() << std::endl;
		for(std::vector<BaseVertex*>::const_iterator pos=m_vtVertexList.begin(); pos!=m_vtVertexList.end();++pos)
		{
			(*pos)->PrintOut(out_stream);
			out_stream << "->";
		}
		out_stream << std::endl <<  "*********************************************" << std::endl;
	}
};
