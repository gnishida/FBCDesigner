/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#include "VBOPm.h"
#include "Polygon3D.h"

#include <qdir.h>
#include <QStringList>
#include <QTime>

#include "VBOPmBlocks.h"
#include "VBOPmParcels.h"
#include "VBOPmBuildings.h"
#include "BlockSet.h"
#include "VBOGeoBuilding.h"

#include "VBOVegetation.h"

///////////////////////////////////////////////////////////////
// INIT
///////////////////////////////////////////////////////////////

// LC
bool VBOPm::initializedLC=false;
static std::vector<QString> sideWalkFileNames;
static std::vector<QVector3D> sideWalkScale;
static std::vector<QString> grassFileNames;

void VBOPm::initLC(){
	QString pathName="../data/textures/LC";
	// 3. sidewalk
	QDir directorySW(pathName+"/sidewalk/");
	QStringList nameFilter;
	nameFilter << "*.png" << "*.jpg" << "*.gif";
	QStringList list = directorySW.entryList( nameFilter, QDir::Files );
	for(int lE=0;lE<list.size();lE++){
		if(QFile::exists(pathName+"/sidewalk/"+list[lE])){
			sideWalkFileNames.push_back(pathName+"/sidewalk/"+list[lE]);
			QStringList scaleS=list[lE].split("_");
			if(scaleS.size()!=4)
				sideWalkScale.push_back(QVector3D(1.0f,1.0f,0));
			else{
				sideWalkScale.push_back(QVector3D(scaleS[1].toFloat(),scaleS[2].toFloat(),0));
			}
		}
	}
	grassFileNames.push_back("../data/textures/LC/grass/grass01.jpg");
	grassFileNames.push_back("../data/textures/LC/grass/grass02.jpg");
	grassFileNames.push_back("../data/textures/LC/grass/grass03.jpg");
	grassFileNames.push_back("../data/textures/LC/grass/grass04.jpg");
	printf("-->Initialized LC\n");
	initializedLC=true;
}


///////////////////////////////////////////////////////////////
// GENERATE 3D GEOMETRY
///////////////////////////////////////////////////////////////

bool VBOPm::generateBlocks(VBORenderManager& rendManager,RoadGraph &roadGraph, BlockSet& blocks, PlaceTypesMainClass& placeTypes){
	//////////////////////////////////////////////
	// INIT
	if(initializedLC==false){//&&G::global().getInt("3d_render_mode")==0){
		initLC();//init LC textures
	}
	QTime timer;

	//////////////////////////////////////////////
	// 1. BLOCKS
	//std::vector< Block > blocks;
	if(VBOPmBlocks::generateBlocks(placeTypes,roadGraph,blocks)==false){
		printf("ERROR: generateBlocks\n");
		return false;
	}
	printf(">>Num Blocks %d\n",blocks.blocks.size());
	
	//blocks.generateMesh(rendManager);
	return true;
}//


bool VBOPm::generateParcels(VBORenderManager& rendManager, BlockSet& blocks, PlaceTypesMainClass& placeTypes) {
	if(VBOPmParcels::generateParcels(placeTypes, blocks.blocks)==false){
		printf("ERROR: generateParcels\n");
		return false;
	}
	printf(">>Parcels were generated.\n");
	
	//generateBlockMesh(rendManager, blocks);
	
	return true;
}

bool VBOPm::generateBuildings(VBORenderManager& rendManager, BlockSet& blocks, PlaceTypesMainClass& placeTypes) {
	QTime timer;

	//////////////////////////////////////////////
	// 4. BUILDINGS
	if(VBOPmBuildings::generateBuildings(placeTypes, blocks.blocks)==false){
		printf("ERROR: generateBuildings\n");
		return false;
	}
	printf(">>Buildings contours were generated.\n");
	
	//////////////////////////////////////////////
	// 5. BUILDINGS Geometry
	rendManager.removeStaticGeometry("3d_building");
	rendManager.removeStaticGeometry("3d_building_fac");
	
	timer.start();
	
	Block::parcelGraphVertexIter vi, viEnd;
	for(int bN=0;bN<blocks.size();bN++){
		if(blocks[bN].isPark==true) continue;//skip those with parks
		for(boost::tie(vi, viEnd) = boost::vertices(blocks[bN].myParcels); vi != viEnd; ++vi) {
			VBOGeoBuilding::generateBuilding(rendManager,blocks[bN].myParcels[*vi].myBuilding, 1);
		}
	}
	printf("Building generation: %d ms\n",timer.elapsed());

	return true;
}

bool VBOPm::generateVegetation(VBORenderManager& rendManager, BlockSet& blocks, PlaceTypesMainClass& placeTypes) {
	VBOVegetation::generateVegetation(rendManager, placeTypes, blocks.blocks);

	return true;
}

void VBOPm::generateBlockMesh(VBORenderManager& rendManager, BlockSet& blocks) {
	if(initializedLC==false){//&&G::global().getInt("3d_render_mode")==0){
		initLC();//init LC textures
	}

	rendManager.removeStaticGeometry("3d_sidewalk");
	rendManager.removeStaticGeometry("3d_block");
	rendManager.removeStaticGeometry("3d_parcel");

	for (int i = 0; i < blocks.size(); ++i) {
		// blocks
		{
			std::vector<Vertex> vert;

			QVector3D color;
			if (i == blocks.selectedBlockIndex) {
				color = QVector3D(1.0f, 1.0f, 1.0f);
			} else if (blocks[i].isPark) {
				color = QVector3D(0.8f, 0.8f, 0.0f);
			} else {
				color = QVector3D(0.0f, 0.5f, 0.8f);
			}

			for(int sN=0;sN<blocks[i].blockContour.contour.size();sN++){
				int next = (sN+1) % blocks[i].blockContour.contour.size();
				vert.push_back(Vertex(QVector3D(blocks[i].blockContour.contour[sN].x(),blocks[i].blockContour.contour[sN].y(), 1),
					color,QVector3D(),QVector3D()));
				vert.push_back(Vertex(QVector3D(blocks[i].blockContour.contour[next].x(),blocks[i].blockContour.contour[next].y(), 1),
					color,QVector3D(),QVector3D()));
			}
			//rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_POINTS,1|mode_AdaptTerrain);
			rendManager.addStaticGeometry("3d_block",vert,"",GL_LINES,1|mode_AdaptTerrain);
		}
	
		// parcels
		if (!blocks[i].isPark) {
			Block::parcelGraphVertexIter vi, viEnd;
			
			int cnt = 0;
			for (boost::tie(vi, viEnd) = boost::vertices(blocks[i].myParcels); vi != viEnd; ++vi, ++cnt) {
				std::vector<Vertex> vert;
				QVector3D color;

				if (i == blocks.selectedBlockIndex && cnt == blocks.selectedParcelIndex) {
					color = QVector3D(1.0f, 1.0f, 1.0f);
				} else if (blocks[i].myParcels[*vi].parcelType == PAR_PARK) {
					color = QVector3D(0.8f, 0.8f, 0.0f);
				} else {
					color = QVector3D(0.0f, 0.5f, 0.8f);
				}

				for (int j = 0; j < blocks[i].myParcels[*vi].parcelContour.contour.size(); ++j) {
					int next = (j+1) % blocks[i].myParcels[*vi].parcelContour.contour.size();

					vert.push_back(Vertex(QVector3D(blocks[i].myParcels[*vi].parcelContour.contour[j].x(), blocks[i].myParcels[*vi].parcelContour.contour[j].y(), 2), color, QVector3D(), QVector3D()));
					vert.push_back(Vertex(QVector3D(blocks[i].myParcels[*vi].parcelContour.contour[next].x(), blocks[i].myParcels[*vi].parcelContour.contour[next].y(), 2), color, QVector3D(), QVector3D()));
				}
			
				rendManager.addStaticGeometry("3d_parcel", vert, "", GL_LINES, 1|mode_AdaptTerrain);
			}
		}
		
		// sidewalk
		{
			if (blocks[i].isPark==true) {
				// PARK
				int randPark=qrand()%grassFileNames.size();
				rendManager.addStaticGeometry2("3d_sidewalk",blocks[i].blockContour.contour,0.5f,false,grassFileNames[randPark],GL_QUADS,2|mode_AdaptTerrain,QVector3D(0.05f,0.05f,0.05f),QVector3D());
			} else {
				// NORMAL
				int randSidewalk=qrand()%sideWalkFileNames.size();

				rendManager.addStaticGeometry2("3d_sidewalk",blocks[i].blockContour.contour,1.5f,false,sideWalkFileNames[randSidewalk],GL_QUADS,2|mode_AdaptTerrain,sideWalkScale[randSidewalk],QVector3D());
				//sides
				std::vector<Vertex> vert;
				for(int sN=0;sN<blocks[i].blockContour.contour.size();sN++){
					int ind1 = sN;
					int ind2 = (sN+1) % blocks[i].blockContour.contour.size();
					QVector3D dir = blocks[i].blockContour.contour[ind2] - blocks[i].blockContour.contour[ind1];
					float length = dir.length();
					dir /= length;
					//printf("z %f\n",blocks[bN].blockContour.contour[ind1].z());
					QVector3D p1 = blocks[i].blockContour.contour[ind1]+QVector3D(0,0, 1.0f);
					QVector3D p2 = blocks[i].blockContour.contour[ind2]+QVector3D(0,0, 1.0f);
					QVector3D p3 = blocks[i].blockContour.contour[ind2]+QVector3D(0,0, 1.5f);
					QVector3D p4 = blocks[i].blockContour.contour[ind1]+QVector3D(0,0, 1.5f);
					QVector3D normal = QVector3D::crossProduct(p2-p1,p4-p1).normalized();
					vert.push_back(Vertex(p1,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p2,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p3,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p4,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
				}
				rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_QUADS,1|mode_Lighting|mode_AdaptTerrain);
			}
		}
	}
}
