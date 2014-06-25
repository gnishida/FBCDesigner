/************************************************************************************************
 *		Procedural City Generation: Parcel
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include "VBOPlaceType.h"
#include "VBOBlock.h"

class VBOPmParcels{
public:

	static bool generateParcels(
		PlaceTypesMainClass &placeTypesIn,
		std::vector< Block > &blocks);

	static void assignPlaceTypeToParcels(PlaceTypesMainClass &placeTypesIn, std::vector< Block > &blocks);
};
