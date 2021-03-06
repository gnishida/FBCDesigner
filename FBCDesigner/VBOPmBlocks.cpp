﻿/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#include "VBOPmBlocks.h"
#include "Polygon3D.h"

#include <qdir.h>
#include <QStringList>
#include "GraphUtil.h"
#include "Util.h"
#include "VBOPmParcels.h"

///////////////////////////////////////////////////////////////
// BLOCKS
///////////////////////////////////////////////////////////////
RoadGraph * roadGraphPtr;
std::vector< Block > * blocksPtr;

Polygon3D sidewalkContourTmp;
Polygon3D sidewalkContourPoints;
std::vector<Polyline3D> sidewalkContourLines;

std::vector< float > sidewalkContourWidths;
bool isFirstVertexVisited;

int curRandSeed;
int curPlaceTypeIdx;

int face_index = 0;
bool vertex_output_visitor_invalid = false;

struct output_visitor : public boost::planar_face_traversal_visitor
{
	void begin_face()
	{
		//std::cout << "face: " << face_index++ << std::endl;
		
		sidewalkContourTmp.clear();
		sidewalkContourWidths.clear();

		sidewalkContourPoints.clear();
		sidewalkContourLines.clear();

		vertex_output_visitor_invalid = false;
	}

	void end_face()
	{
		sidewalkContourTmp.clear();

		if (vertex_output_visitor_invalid){ 
			printf("INVALID end face\n");
			return;
		}
			
		for (int i = 0; i < sidewalkContourPoints.contour.size(); ++i) {
			sidewalkContourTmp.push_back(sidewalkContourPoints[i]);
			//sidewalkContourTmp.contour.back().setZ(0);//forze height =0

			if ((sidewalkContourLines[i][0] - sidewalkContourPoints[i]).lengthSquared() < (sidewalkContourLines[i].last() - sidewalkContourPoints[i]).lengthSquared()) {
				for (int j = 1; j < sidewalkContourLines[i].size() - 1; ++j) {
					sidewalkContourTmp.push_back(sidewalkContourLines[i][j]);
					//blockContourTmp.contour.back().setZ(0);//forze height =0
				}
			} else {
				for (int j = sidewalkContourLines[i].size() - 2; j > 0; --j) {
					sidewalkContourTmp.push_back(sidewalkContourLines[i][j]);
					//blockContourTmp.contour.back().setZ(0);//forze height =0
				}
			}
		}

		//if (blockContourTmp.area() > 100.0f) {
		if (sidewalkContourTmp.contour.size() >= 3 && sidewalkContourWidths.size() >= 3) {
			Block newBlock;
			newBlock.sidewalkContour = sidewalkContourTmp;
			newBlock.sidewalkContourRoadsWidths = sidewalkContourWidths;
			while (newBlock.sidewalkContour.contour.size() > newBlock.sidewalkContourRoadsWidths.size()) {
				newBlock.sidewalkContourRoadsWidths.push_back(newBlock.sidewalkContourRoadsWidths.back());
			}
	
			blocksPtr->push_back(newBlock);
			//printf("CREATE block %d: %d\n",blocksPtr->size(),blocksPtr->back().blockContour.contour.size());
		}else{
			printf("Contour %d widths %d\n",sidewalkContourTmp.contour.size(),sidewalkContourWidths.size());
		}
	}
};

//Vertex visitor
struct vertex_output_visitor : public output_visitor
{
	template <typename Vertex> 
	void next_vertex(Vertex v) 
	{ 	
		if (v >= boost::num_vertices(roadGraphPtr->graph)) {
			vertex_output_visitor_invalid = true;
			printf("INVALID vertex\n");
			return;
		}
		//std::cout << v << " 
		/*
		if(  v >= 0 && v < boost::num_vertices(roadGraphPtr->graph) ){
			blockContourTmp.push_back( (roadGraphPtr->graph)[v]->pt );

			//initialize block random seed from first street node random seed
			if(isFirstVertexVisited){
				isFirstVertexVisited = false;
				curRandSeed = ( (roadGraphPtr->graph)[v]->randSeed*4096 + 150889) % 714025;
			}
		}*/
		sidewalkContourPoints.push_back(roadGraphPtr->graph[v]->pt);
	}

	template <typename Edge> 
	void next_edge(Edge e) 
	{ 
		RoadVertexDesc src = boost::source(e, roadGraphPtr->graph);
		RoadVertexDesc tgt = boost::target(e, roadGraphPtr->graph);
		if (src >= boost::num_vertices(roadGraphPtr->graph) || tgt >= boost::num_vertices(roadGraphPtr->graph)) {
			vertex_output_visitor_invalid = true;
			printf("INVALID edge\n");
			return;
		}

		sidewalkContourLines.push_back(roadGraphPtr->graph[e]->polyline3D);

		for (int i = 0; i < roadGraphPtr->graph[e]->polyline3D.size() - 1; ++i) {
			sidewalkContourWidths.push_back(0.5f * roadGraphPtr->graph[e]->getWidth());
		}

	}
};

//
// Remove intersecting edges of a graph
// GEN: This function considers each edge as a straight line segment, instead of a polyline.
//      This may cause unnecessary removal of edges, but it usually can guarantee the planar graph after this process, so I will go with this.
//
bool removeIntersectingEdges(RoadGraph &roadGraph)
{
	//QSet<RoadGraph::roadGraphEdgeIter*> edgesToRemove2;
	std::vector<RoadEdgeIter> edgesToRemove;

	QVector2D a0, a1, b0, b1;
	QVector2D intPt;
	RoadEdgeIter a_ei, a_ei_end;
	RoadEdgeIter b_ei, b_ei_end;
	float ta0a1, tb0b1;

	for(boost::tie(a_ei, a_ei_end) = boost::edges(roadGraph.graph); a_ei != a_ei_end; ++a_ei){
		a0 = QVector2D(roadGraph.graph[boost::source(*a_ei,roadGraph.graph)]->pt);
		a1 = QVector2D(roadGraph.graph[boost::target(*a_ei,roadGraph.graph)]->pt);

		//for(tie(b_ei, b_ei_end) = boost::edges(roadGraph.graph); b_ei != b_ei_end; ++b_ei){
		for(b_ei = a_ei; b_ei != a_ei_end; ++b_ei){			

			if(b_ei != a_ei){
				b0 = QVector2D(roadGraph.graph[boost::source(*b_ei,roadGraph.graph)]->pt);
				b1 = QVector2D(roadGraph.graph[boost::target(*b_ei,roadGraph.graph)]->pt);

				if(Polygon3D::segmentSegmentIntersectXY(a0, a1, b0, b1, &ta0a1, &tb0b1, true, intPt) ){
					bool addEd=true;
					for(int eN=0;eN<edgesToRemove.size();eN++){
						if(edgesToRemove[eN]==b_ei){
							addEd=false;
							break;
						}
					}
					if(addEd)
						edgesToRemove.push_back(b_ei);
					/*// remove other as well
					addEd=true;
					for(int eN=0;eN<edgesToRemove.size();eN++){
						if(edgesToRemove[eN]==a_ei){
							addEd=false;
							break;
						}
					}
					if(addEd)
						edgesToRemove.push_back(a_ei);
					///*/
				}
			}
		}		
	}

	for(int i=0; i<edgesToRemove.size(); ++i){
		boost::remove_edge(*(edgesToRemove[i]),roadGraph.graph);
	}

	if(edgesToRemove.size()>0){
		printf("Edge removed %d\n",edgesToRemove.size());
		return true;
	} else {
		return false;
	}
}//


/**
 * 道路網から、Block情報を抽出する。
 */
bool VBOPmBlocks::generateBlocks(PlaceTypesMainClass &placeTypesIn, RoadGraph &roadGraph, BlockSet &blocks) {
	std::cout << "normalizing loop of roads." << std::endl;
	GraphUtil::normalizeLoop(roadGraph);
	std::cout << "normalizing done." << std::endl;

	roadGraphPtr = &roadGraph;
	blocksPtr = &blocks.blocks;
	blocksPtr->clear();

	bool isPlanar = false;
	bool converges = true;

	//GraphUtil::planarify(roadGraph);
	//GraphUtil::clean(roadGraph);
	
	//Make sure graph is planar
	typedef std::vector< RoadEdgeDesc > tEdgeDescriptorVector;
	std::vector<tEdgeDescriptorVector> embedding(boost::num_vertices(roadGraph.graph));

	int cont=0;

	// Test for planarity
	while (cont<2) {
		std::cout << "planarity_test..." << std::endl;
		if (boost::boyer_myrvold_planarity_test(boost::boyer_myrvold_params::graph =roadGraph.graph,
			boost::boyer_myrvold_params::embedding = &embedding[0]) 
			){
				isPlanar = true;
				break;
		} else {
			std::cout << "Input graph is not planar trying removeIntersectingEdges" << std::endl;
			// No planar: Remove intersecting edges and check again
			removeIntersectingEdges(roadGraph);
			cont++;
		}
	}

	if (!isPlanar) {
		std::cout << "ERROR: Graph could not be planarized: (generateBlocks)\n";
		return false;
	}
	
	// build embedding manually
	//embedding.clear();
	//embedding.resize(boost::num_vertices(roadGraph.graph));
	buildEmbedding(roadGraph, embedding);

	//Create edge index property map?	
	typedef std::map<RoadEdgeDesc, size_t> EdgeIndexMap;
	EdgeIndexMap mapEdgeIdx;
	boost::associative_property_map<EdgeIndexMap> pmEdgeIndex(mapEdgeIdx);		
	RoadEdgeIter ei, ei_end;	
	int edge_count = 0;
	for(boost::tie(ei, ei_end) = boost::edges(roadGraph.graph); ei != ei_end; ++ei){
		mapEdgeIdx.insert(std::make_pair(*ei, edge_count++));	
	}
	std::cout << "edge index was built." << std::endl;

	//Extract blocks from road graph using boost graph planar_face_traversal
	vertex_output_visitor v_vis;	
	boost::planar_face_traversal(roadGraph.graph, &embedding[0], v_vis, pmEdgeIndex);
	std::cout << "faces were traversed." << std::endl;

	//Misc postprocessing operations on blocks =======
	int maxVtxCount = 0;
	int maxVtxCountIdx = -1;
	std::vector<float> blockAreas;

	Loop3D sidewalkContourInset;
	for (int i = 0; i < blocks.size(); ++i) {
		//Reorient faces
		if (Polygon3D::reorientFace(blocks[i].sidewalkContour.contour)) {
			std::reverse(blocks[i].sidewalkContourRoadsWidths.begin(), blocks[i].sidewalkContourRoadsWidths.end() - 1);
		}

		if( blocks[i].sidewalkContour.contour.size() != blocks[i].sidewalkContourRoadsWidths.size() ){
			std::cout << "Error: contour" << blocks[i].sidewalkContour.contour.size() << " widhts " << blocks[i].sidewalkContourRoadsWidths.size() << "\n";
			blocks[i].sidewalkContour.contour.clear();
			blockAreas.push_back(0.0f);
			continue;
		}

		if(blocks[i].sidewalkContour.contour.size() < 3){
			std::cout << "Error: Contour <3 " << "\n";
			blockAreas.push_back(0.0f);
			continue;
		}

		//Compute block offset	
		float insetArea = blocks[i].sidewalkContour.computeInset(blocks[i].sidewalkContourRoadsWidths,sidewalkContourInset);
		
		blocks[i].sidewalkContour.contour = sidewalkContourInset;
		blocks[i].sidewalkContour.getBBox3D(blocks[i].bbox.minPt, blocks[i].bbox.maxPt);
		
		blockAreas.push_back(insetArea);
	}

	//Remove the largest block
	float maxArea = -FLT_MAX;
	int maxAreaIdx = -1;
	for(int i=0; i<blocks.size(); ++i){
		if(blocks[i].sidewalkContour.contour.size() < 3){
			continue;
		}
		if(blockAreas[i] > maxArea){
			maxArea = blockAreas[i];
			maxAreaIdx = i;
		}
	}
	if(maxAreaIdx != -1){
		blocks.blocks.erase(blocks.blocks.begin()+maxAreaIdx);
		blockAreas.erase(blockAreas.begin()+maxAreaIdx);
	}

	assignPlaceTypeToBlocks(placeTypesIn, blocks);

	// block park
	for (int i = 0; i < blocks.size(); ++i) {
		if (Util::genRand() < placeTypesIn.myPlaceTypes[blocks[i].getMyPlaceTypeIdx()].getFloat("park_percentage")) {
			blocks[i].isPark = true;
		}
	}

	// 歩道の分を確保するため、ブロックを縮小する。
	for (int i = 0; i < blocks.size(); ++i) {
		Loop3D blockContourInset;
		float sidewalk_width = placeTypesIn.myPlaceTypes[blocks[i].getMyPlaceTypeIdx()].getFloat("sidewalk_width");
		blocks[i].sidewalkContour.computeInset(sidewalk_width, blockContourInset, false);
		blocks[i].blockContour.contour = blockContourInset;
		blocks[i].blockContour.getBBox3D(blocks[i].bbox.minPt, blocks[i].bbox.maxPt);
	}

	return true;
}

void VBOPmBlocks::buildEmbedding(RoadGraph &roads, std::vector<std::vector<RoadEdgeDesc> > &embedding) {
	std::cout << "building embedding..." << std::endl;
	time_t start = clock();

	for (int i = 0; i < embedding.size(); ++i) {
		QMap<float, RoadEdgeDesc> edges;

		for (int j = 0; j < embedding[i].size(); ++j) {
			Polyline2D polyline = GraphUtil::orderPolyLine(roads, embedding[i][j], i);
			QVector2D vec = polyline[1] - polyline[0];
			edges[-atan2f(vec.y(), vec.x())] = embedding[i][j];
		}

		std::vector<RoadEdgeDesc> edge_descs;
		for (QMap<float, RoadEdgeDesc>::iterator it = edges.begin(); it != edges.end(); ++it) {
			edge_descs.push_back(it.value());
		}

		embedding[i] = edge_descs;
	}

	time_t end = clock();
	std::cout << "embedding was built. Elapsed time: " << (double)(end-start)/CLOCKS_PER_SEC << "[sec]" << std::endl;
}

void VBOPmBlocks::assignPlaceTypeToBlocks(PlaceTypesMainClass &placeTypesIn, BlockSet& blocks) {
	bool useSamePlaceTypeForEntireBlock = false;

	for (int i = 0; i < blocks.size(); ++i) {
		// assign placetype to the block
		{
			QVector3D testPt = blocks[i].bbox.midPt();

			int validClosestPlaceTypeIdx = -1;
			for (int k = 0; k < placeTypesIn.size(); ++k) {
				if (placeTypesIn.myPlaceTypes[k].containsPoint(QVector2D(testPt))) {
					validClosestPlaceTypeIdx = k;
					break;
				}
			}
			blocks[i].setMyPlaceTypeIdx(validClosestPlaceTypeIdx);
		}

		// assign placetype to each parcel
		VBOPmParcels::assignPlaceTypeToParcels(placeTypesIn, blocks[i]);
	}
}