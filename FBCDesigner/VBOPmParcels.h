/************************************************************************************************
 *		Procedural City Generation: Parcel
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include "VBOPlaceType.h"
#include "VBOBlock.h"

class VBOPmParcels{
public:

	static bool generateParcels(VBORenderManager& rendManager, PlaceTypesMainClass &placeTypesIn, std::vector< Block > &blocks);

	static void assignPlaceTypeToParcels(PlaceTypesMainClass &placeTypesIn, Block& blocks);
};
