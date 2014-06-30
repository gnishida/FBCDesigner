/************************************************************************************************
*		VBO Block Class
*		@author igarciad
************************************************************************************************/
#pragma once

#ifndef Q_MOC_RUN
#include <boost/graph/adjacency_list.hpp>
#endif

#include "VBORenderManager.h"
#include "VBOParcel.h"
#include <QVector3D>
#include "Polygon3D.h"

/**
* Block.
**/

class Block {
public:
	/**
	* BGL Graph of parcels into which block is subdivided.
	**/				 

	typedef boost::adjacency_list
		<boost::vecS, boost::vecS, boost::undirectedS, Parcel> parcelGraph;				

	typedef boost::graph_traits<parcelGraph>::vertex_descriptor parcelGraphVertexDesc;

	typedef boost::graph_traits<parcelGraph>::vertex_iterator parcelGraphVertexIter;

	typedef boost::graph_traits<parcelGraph>::edge_iterator parcelGraphEdgeIter;

	typedef boost::graph_traits<parcelGraph>::adjacency_iterator parcelGraphAdjIter;// Carlos


private:
	int myPlaceTypeIdx;
	int myNeighborhoodID;
	int myCityID;

public:
	parcelGraph myParcels;

	//QVector3D myColor;

	BBox3D bbox;

	int randSeed;
	bool isPark;

	/**
	* Contour of the block.
	**/
	Polygon3D blockContour;

	/**
	* Boundary road widths
	**/
	std::vector<float> blockContourRoadsWidths;

public:
	/**
	* Constructor.
	**/
	Block() : myPlaceTypeIdx(-1), isPark(false) {}

	/**
	* Destructor.
	**/
	~Block() {}

	/**
	* Clear
	**/
	void clear(void);

	void computeMyBBox3D(void);

	inline void setMyPlaceTypeIdx(int inIdx)
	{
		myPlaceTypeIdx = inIdx;
	}

	inline int getMyPlaceTypeIdx(void)
	{
		return myPlaceTypeIdx;
	}

	inline int getMyNeighborhoodID(void)
	{
		return myNeighborhoodID;
	}

	inline int getMyCityID(void)
	{						
		return myCityID;
	}


	/**
	* Compute parcel adjacency graph
	**/
	void computeParcelAdjacencyGraph(void);

	void buildableAreaMock(void);

	static void findParcelFrontAndBackEdges(Block &inBlock, Parcel &inParcel,
		std::vector<int> &frontEdges,
		std::vector<int> &rearEdges,
		std::vector<int> &sideEdges );



	/**
	* Adapt block to vboRenderManager
	**/
	//void adaptBlockToTerrain(MTC::geometry::ElevationGrid *elGrid);


	bool splitBlockParcelsWithRoadSegment(std::vector<QVector3D> &roadSegmentGeometry,
		float roadSegmentWidth, BBox3D roadSegmentBBox3D, std::list<Parcel> &blockParcels);

	bool areParcelsAdjacent(parcelGraphVertexIter &p0, parcelGraphVertexIter &p1);
	
	//void generateMesh(VBORenderManager& rendManager);
	void adaptToTerrain(VBORenderManager* vboRenderManager);
};

