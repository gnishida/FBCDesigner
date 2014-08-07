/************************************************************************************************
 *		Procedural City Generation: Parcel
 *		@author igarciad
 ************************************************************************************************/

#include "VBOPmParcels.h"
#include "qmatrix4x4.h"


void subdivideBlockIntoParcels(Block &block, PlaceTypesMainClass &placeTypesIn);
bool subdivideParcel(Block &block, Parcel parcel, float areaMean, float areaMin, float areaVar, float splitIrregularity, std::vector<Parcel> &outParcels); 
void setParcelsAsParks(PlaceTypesMainClass &placeTypesIn, std::vector< Block > &blocks);

bool VBOPmParcels::generateParcels(VBORenderManager& rendManager, PlaceTypesMainClass &placeTypesIn, std::vector< Block > &blocks) {
	std::cout << "start #"<<blocks.size()<<"...";
	for (int i = 0; i < blocks.size(); ++i) {
		subdivideBlockIntoParcels(blocks[i], placeTypesIn);

		if (placeTypesIn.size() > 0) {
			assignPlaceTypeToParcels(placeTypesIn, blocks[i]);
		}

		blocks[i].adaptToTerrain(&rendManager);
	}
	std::cout << "end...";

	if (placeTypesIn.size() > 0) {
		setParcelsAsParks(placeTypesIn, blocks);
	}

	return true;
}

void subdivideBlockIntoParcels(Block &block, PlaceTypesMainClass &placeTypesIn) {
	srand(block.randSeed);
	std::vector<Parcel> tmpParcels;

	//Empty parcels in block
	block.myParcels.clear();

	//Make the initial parcel of the block be the block itself
	//Parcel
	Parcel tmpParcel;
	tmpParcel.setContour(block.blockContour);
	//std::cout << block.myPlaceTypeIdx << " "; std::fflush(stdout);

	if( block.getMyPlaceTypeIdx() == -1){
		tmpParcels.push_back(tmpParcel);
	} else {
		//start recursive subdivision
		subdivideParcel(block, tmpParcel,
			placeTypesIn.myPlaceTypes.at(block.getMyPlaceTypeIdx()).getFloat("parcel_area_mean"),
			placeTypesIn.myPlaceTypes.at(block.getMyPlaceTypeIdx()).getFloat("parcel_area_min"),
			placeTypesIn.myPlaceTypes.at(block.getMyPlaceTypeIdx()).getFloat("parcel_area_deviation"),
			placeTypesIn.myPlaceTypes.at(block.getMyPlaceTypeIdx()).getFloat("parcel_split_deviation"),
			tmpParcels);
	}

	//printf("Block subdivided into %d parcels\n", tmpParcels.size());
	//Add parcels to block graph and compute adjacencies
	Block::parcelGraphVertexDesc tmpPGVD;
	//std::cout <<tmpParcels.size() <<"\n";
	for(int i=0; i<tmpParcels.size(); ++i){
		//assign index of place type to parcel
		tmpParcels[i].setMyPlaceTypeIdx( block.getMyPlaceTypeIdx() );
		//add parcel to block parcels graph
		tmpPGVD = boost::add_vertex(block.myParcels);
		block.myParcels[tmpPGVD] = tmpParcels[i];
		//std::cout << "h2 ";	std::fflush(stdout);
	}
}//

/**
* Parcel subdivision
* @desc: Recursive subdivision of a parcel using OBB technique
* @return: true if parcel was successfully subdivided, false otherwise
* @areaMean: mean target area of parcels after subdivision
* @areaVar: variance of parcels area after subdivision (normalized in 0-1)
* @splitIrregularity: A normalized value 0-1 indicating how far
*					from the middle point the split line should be
**/
bool subdivideParcel(Block &block, Parcel parcel, float areaMean, float areaMin, float areaStd,
	float splitIrregularity, std::vector<Parcel> &outParcels)
{
	//printf("subdivideParcel\n");
	//check if parcel is subdividable
	float thresholdArea = areaMean + areaStd*areaMean*(((float)qrand()/RAND_MAX)*2.0f-1.0f);//LC::misctools::genRand(-1.0f, 1.0f)
	//float thresholdArea = areaMean + LC::misctools::genRand(0.0f, 1.0f)*areaStd;
	if( (fabs(boost::geometry::area(parcel.bg_parcelContour))) <= std::max(thresholdArea, areaMin)) {
		//printf("a: %.3f %.3f", boost::geometry::area(parcel.bg_parcelContour));
		//boost::geometry::correct(parcel.bg_parcelContour);
		//printf("a: %.3f %.3f", boost::geometry::area(parcel.bg_parcelContour));
		outParcels.push_back(parcel);
		return false;
	}

	//compute OBB
	QVector3D obbSize;
	QMatrix4x4 obbMat;
	parcel.parcelContour.getMyOBB(obbSize, obbMat);

	//compute split line passing through center of OBB TODO (+/- irregularity)
	//		and with direction parallel/perpendicular to OBB main axis
	QVector3D slEndPoint;
	QVector3D dirVectorInit, dirVector, dirVectorOrthogonal;
	QVector3D midPt(0.0f, 0.0f, 0.0f);
	QVector3D auxPt(1.0f, 0.0f, 0.0f);
	QVector3D midPtNoise(0.0f, 0.0f, 0.0f);
	std::vector<QVector3D> splitLine;	

	midPt = midPt*obbMat;

	dirVectorInit = (auxPt*obbMat - midPt);
	dirVectorInit.normalize();
	if(obbSize.x() > obbSize.y()){
		dirVector.setX( -dirVectorInit.y() );
		dirVector.setY(  dirVectorInit.x() );
	} else {
		dirVector = dirVectorInit;
	}

	midPtNoise.setX( splitIrregularity*(((float)qrand()/RAND_MAX)*20.0f-10.0f));//LC::misctools::genRand(-10.0f, 10.0f) );
	midPtNoise.setY( splitIrregularity*(((float)qrand()/RAND_MAX)*20.0f-10.0f));//LC::misctools::genRand(-10.0f, 10.0f) );
	midPt = midPt + midPtNoise;

	slEndPoint = midPt + 10000.0f*dirVector;
	splitLine.push_back(slEndPoint);
	slEndPoint = midPt - 10000.0f*dirVector;
	splitLine.push_back(slEndPoint);

	//split parcel with line and obtain two new parcels
	Polygon3D pgon1, pgon2;

	float kDistTol = 0.01f;

	if (parcel.parcelContour.splitMeWithPolyline(splitLine, pgon1.contour, pgon2.contour)) {
		Parcel parcel1;
		Parcel parcel2;

		parcel1.setContour(pgon1);
		parcel2.setContour(pgon2);

		//call recursive function for both parcels
		subdivideParcel(block, parcel1, areaMean, areaMin, areaStd, splitIrregularity, outParcels);
		subdivideParcel(block, parcel2, areaMean, areaMin, areaStd, splitIrregularity, outParcels);
	} else {
		return false;
	}

	return true;
}

bool compareFirstPartTuple (const std::pair<float,Parcel*> &i, const std::pair<float,Parcel*> &j) {
	return (i.first<j.first);
}


void VBOPmParcels::assignPlaceTypeToParcels(PlaceTypesMainClass &placeTypesIn, Block& block) {
	bool useSamePlaceTypeForEntireBlock = false;
	
	Block::parcelGraphVertexIter vi, viEnd;
	for (boost::tie(vi, viEnd) = boost::vertices(block.myParcels); vi != viEnd; ++vi) {
		if (useSamePlaceTypeForEntireBlock) {
			block.myParcels[*vi].setMyPlaceTypeIdx(block.getMyPlaceTypeIdx());
		} else {
			QVector3D testPt = block.myParcels[*vi].bbox.midPt();

			int validClosestPlaceTypeIdx = -1;
			for (int k = 0; k < placeTypesIn.size(); ++k) {
				if (placeTypesIn.myPlaceTypes[k].containsPoint(QVector2D(testPt))) {
					validClosestPlaceTypeIdx = k;
					break;
				}					
			}
			block.myParcels[*vi].setMyPlaceTypeIdx(validClosestPlaceTypeIdx);
		}
	}
}

void setParcelsAsParks(PlaceTypesMainClass &placeTypesIn, std::vector< Block > &blocks) {
	for (int k = 0; k < placeTypesIn.size(); ++k) {
		std::vector<Parcel*> parcelPtrs;

		bool isFirst = true;
		int seedOfFirstBlock = 0;

		//get all the parcels of that place type in an array
		for(int j=0; j<blocks.size(); ++j){			
			Block::parcelGraphVertexIter vi, viEnd;
			for (boost::tie(vi, viEnd) = boost::vertices(blocks.at(j).myParcels); vi != viEnd; ++vi) {
				if (blocks.at(j).myParcels[*vi].getMyPlaceTypeIdx() == k) {
					if(isFirst){
						seedOfFirstBlock = blocks.at(j).randSeed;
						isFirst = false;
					}

					blocks.at(j).myParcels[*vi].parcelType= PAR_WITH_BLDG;
					parcelPtrs.push_back( &(blocks.at(j).myParcels[*vi]) );

				}
			}
		}

		srand(seedOfFirstBlock);

		float parkPercentage = placeTypesIn.myPlaceTypes[k].getFloat("park_percentage");

		//shuffle and select first parkPercentage %
		int numToSetAsParks = (int)(parkPercentage*( (float)(parcelPtrs.size()) ));
		std::random_shuffle( parcelPtrs.begin(), parcelPtrs.end() );

		int countMax = std::min<float>( parcelPtrs.size(), numToSetAsParks );
		for(int i=0; i < countMax ; ++i){
			(parcelPtrs.at(i))->parcelType=PAR_PARK;
		}
	}
}
