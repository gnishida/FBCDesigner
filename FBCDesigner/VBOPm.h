/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include "VBOBlock.h"
#include "VBOParcel.h"
#include "VBOBuilding.h"
#include "VBOPlaceType.h"
#include "RoadGraph.h"
#include "BlockSet.h"

class VBORenderManager;

class VBOPm
{
public:

	static bool initialized;
	static void init();

	static bool initializedLC;
	static void initLC();

	static PlaceTypesMainClass placeTypes;

	static bool generateBlocks(VBORenderManager& rendManager, RoadGraph &roadGraph, BlockSet& blocks);
	static bool generateParcels(VBORenderManager& rendManager, BlockSet& blocks);
	static bool generateBuildings(VBORenderManager& rendManager, BlockSet& blocks);
	static bool generateVegetation(VBORenderManager& rendManager, BlockSet& blocks);
	static void generateBlockMesh(VBORenderManager& rendManager, BlockSet& block);
private:

};


