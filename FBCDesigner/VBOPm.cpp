﻿/************************************************************************************************
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
#include "Polygon3D.h"
#include "Util.h"
#include "HeatMapColorTable.h"

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


/**
 * 道路網から、ブロック情報を抽出する。
 */
bool VBOPm::generateBlocks(VBORenderManager& rendManager,RoadGraph &roadGraph, BlockSet& blocks, PlaceTypesMainClass& placeTypes){
	//////////////////////////////////////////////
	// INIT
	if(initializedLC==false){//&&G::global().getInt("3d_render_mode")==0){
		initLC();//init LC textures
	}
	QTime timer;

	//////////////////////////////////////////////
	// 1. BLOCKS
	if (!VBOPmBlocks::generateBlocks(placeTypes,roadGraph,blocks)) {
		printf("ERROR: generateBlocks\n");
		return false;
	}
	printf(">>Num Blocks %d\n",blocks.blocks.size());

	// 3Dモデルを生成する
	rendManager.removeStaticGeometry("3d_sidewalk");
	rendManager.removeStaticGeometry("3d_block");
	for (int i = 0; i < blocks.size(); ++i) {
		blocks[i].adaptToTerrain(&rendManager);

		// Blockの3Dモデルを生成（Block表示モードの時にのみ、表示される）
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
				vert.push_back(Vertex(QVector3D(blocks[i].blockContour.contour[sN].x(),blocks[i].blockContour.contour[sN].y(), 1), color, QVector3D(), QVector3D()));
				vert.push_back(Vertex(QVector3D(blocks[i].blockContour.contour[next].x(),blocks[i].blockContour.contour[next].y(), 1), color, QVector3D(), QVector3D()));
			}
			rendManager.addStaticGeometry("3d_block",vert,"",GL_LINES,1);
		}

		// 歩道の3Dモデルを生成（通常の表示モードの時にのみ、表示される）
		{
			if (blocks[i].isPark==true) {
				// PARK
				int randPark=qrand()%grassFileNames.size();
				rendManager.addStaticGeometry2("3d_sidewalk",blocks[i].blockContour.contour,0.0f,false,grassFileNames[randPark],GL_QUADS,2,QVector3D(0.05f,0.05f,0.05f),QVector3D());
			} else {
				// NORMAL
				int randSidewalk=qrand()%sideWalkFileNames.size();

				rendManager.addStaticGeometry2("3d_sidewalk",blocks[i].blockContour.contour,0.3f,false,sideWalkFileNames[randSidewalk],GL_QUADS,2,sideWalkScale[randSidewalk],QVector3D());
				//sides
				std::vector<Vertex> vert;
				for(int sN=0;sN<blocks[i].blockContour.contour.size();sN++){
					int ind1 = sN;
					int ind2 = (sN+1) % blocks[i].blockContour.contour.size();
					QVector3D dir = blocks[i].blockContour.contour[ind2] - blocks[i].blockContour.contour[ind1];
					float length = dir.length();
					dir /= length;
					//printf("z %f\n",blocks[bN].blockContour.contour[ind1].z());
					QVector3D p1 = blocks[i].blockContour.contour[ind1]+QVector3D(0,0, 0.0f);//1.0f);
					QVector3D p2 = blocks[i].blockContour.contour[ind2]+QVector3D(0,0, 0.0f);//1.0f);
					QVector3D p3 = blocks[i].blockContour.contour[ind2]+QVector3D(0,0, 0.3f);//1.5f);
					QVector3D p4 = blocks[i].blockContour.contour[ind1]+QVector3D(0,0, 0.3f);//1.5f);
					QVector3D normal = QVector3D::crossProduct(p2-p1,p4-p1).normalized();
					vert.push_back(Vertex(p1,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p2,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p3,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p4,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
				}
				rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_QUADS,1|mode_Lighting);

				Block::parcelGraphVertexIter vi, viEnd;
			
				int randPark=qrand()%grassFileNames.size();
				for (boost::tie(vi, viEnd) = boost::vertices(blocks[i].myParcels); vi != viEnd; ++vi) {
					if (blocks[i].myParcels[*vi].parcelType == PAR_PARK) {
						//rendManager.addStaticGeometry2("3d_sidewalk", blocks[i].myParcels[*vi].parcelContour.contour,0.0f,false,grassFileNames[randPark],GL_QUADS,2,QVector3D(0.05f,0.05f,0.05f),QVector3D());
					}
				}
			}
		}

		// 歩道の分を確保するため、ブロックを縮小する。
		Loop3D blockContourInset;
		float sidewalk_width = placeTypes.myPlaceTypes[blocks[i].getMyPlaceTypeIdx()].getFloat("sidewalk_width");
		blocks[i].blockContour.computeInset(sidewalk_width, blockContourInset, false);
		blocks[i].blockContour.contour = blockContourInset;
		blocks[i].blockContour.getBBox3D(blocks[i].bbox.minPt, blocks[i].bbox.maxPt);
	}

	return true;
}

/**
 * Block情報から、Parcel情報を計算する。
 */
bool VBOPm::generateParcels(VBORenderManager& rendManager, BlockSet& blocks, PlaceTypesMainClass& placeTypes) {
	if (!VBOPmParcels::generateParcels(rendManager, placeTypes, blocks.blocks)) {
		printf("ERROR: generateParcels\n");
		return false;
	}
	printf(">>Parcels were generated.\n");

	// 3Dモデルを生成する
	rendManager.removeStaticGeometry("3d_parcel");
	for (int i = 0; i < blocks.size(); ++i) {
		blocks[i].adaptToTerrain(&rendManager);
		//if (!blocks[i].isPark) {
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
				
				/*
				for (int j = 0; j < blocks[i].myParcels[*vi].parcelContour.contour.size(); ++j) {
					int next = (j+1) % blocks[i].myParcels[*vi].parcelContour.contour.size();

					vert.push_back(Vertex(QVector3D(blocks[i].myParcels[*vi].parcelContour.contour[j].x(), blocks[i].myParcels[*vi].parcelContour.contour[j].y(), 1), color, QVector3D(), QVector3D()));
					vert.push_back(Vertex(QVector3D(blocks[i].myParcels[*vi].parcelContour.contour[next].x(), blocks[i].myParcels[*vi].parcelContour.contour[next].y(), 1), color, QVector3D(), QVector3D()));
				}
				*/
				//rendManager.addStaticGeometry("3d_parcel", vert, "", GL_LINES, 1);

				int randPark=1;//qrand()%grassFileNames.size();
				rendManager.addStaticGeometry2("3d_parcel",blocks[i].myParcels[*vi].parcelContour.contour,0.5f,false,grassFileNames[randPark],GL_QUADS,2,QVector3D(0.05f,0.05f,0.05f),QVector3D());
				//sides
				for(int sN=0;sN<blocks[i].myParcels[*vi].parcelContour.contour.size();sN++){
					int ind1 = sN;
					int ind2 = (sN+1) % blocks[i].myParcels[*vi].parcelContour.contour.size();
					QVector3D dir = blocks[i].myParcels[*vi].parcelContour.contour[ind2] - blocks[i].myParcels[*vi].parcelContour.contour[ind1];
					float length = dir.length();
					dir /= length;
					//printf("z %f\n",blocks[bN].blockContour.contour[ind1].z());
					QVector3D p1 = blocks[i].myParcels[*vi].parcelContour.contour[ind1]+QVector3D(0,0, 0.0f);//1.0f);
					QVector3D p2 = blocks[i].myParcels[*vi].parcelContour.contour[ind2]+QVector3D(0,0, 0.0f);//1.0f);
					QVector3D p3 = blocks[i].myParcels[*vi].parcelContour.contour[ind2]+QVector3D(0,0, 0.5f);//1.5f);
					QVector3D p4 = blocks[i].myParcels[*vi].parcelContour.contour[ind1]+QVector3D(0,0, 0.5f);//1.5f);
					QVector3D normal = QVector3D::crossProduct(p2-p1,p4-p1).normalized();
					vert.push_back(Vertex(p1,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p4,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p3,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p2,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
				}
				rendManager.addStaticGeometry("3d_parcel",vert,"",GL_QUADS,1|mode_Lighting);
			}
		//}
	}

	if (!VBOPmBuildings::generateBuildings(rendManager, placeTypes, blocks.blocks)) {
		printf("ERROR: generateBuildings\n");
		return false;
	}
	printf(">>Buildings contours were generated.\n");

	// generate population distribution and job distribution
	generatePopulationJobDistribution(blocks);
		
	return true;
}

bool VBOPm::generateBuildings(VBORenderManager& rendManager, BlockSet& blocks, PlaceTypesMainClass& placeTypes) {
	QTime timer;

	rendManager.removeStaticGeometry("3d_building");
	rendManager.removeStaticGeometry("3d_building_fac");
	
	timer.start();
	
	Block::parcelGraphVertexIter vi, viEnd;
	for (int bN = 0; bN < blocks.size(); bN++) {
		if (blocks[bN].isPark) continue;//skip those with parks
		for (boost::tie(vi, viEnd) = boost::vertices(blocks[bN].myParcels); vi != viEnd; ++vi) {
			if (blocks[bN].myParcels[*vi].myBuilding.buildingFootprint.contour.size() < 3) continue;

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

void VBOPm::generatePlaceTypeMesh(VBORenderManager& rendManager, PlaceTypesMainClass& placeTypes) {
	if (!initializedLC) {
		initLC();//init LC textures
	}

	rendManager.removeStaticGeometry("placetype");

	for (int i = 0; i < placeTypes.size(); ++i) {
		std::vector<Vertex> vert;

		QVector3D color = QVector3D(1.0f, 1.0f, 1.0f);

		for (int j = 0; j < placeTypes.myPlaceTypes[i].area.size(); ++j) {
			int next = (j + 1) % placeTypes.myPlaceTypes[i].area.size();

			float z = rendManager.getTerrainHeight(placeTypes.myPlaceTypes[i].area[j].x(), placeTypes.myPlaceTypes[i].area[j].y());
			vert.push_back(Vertex(QVector3D(placeTypes.myPlaceTypes[i].area[j].x(), placeTypes.myPlaceTypes[i].area[j].y(), z + 2), color, QVector3D(), QVector3D()));
			z = rendManager.getTerrainHeight(placeTypes.myPlaceTypes[i].area[next].x(), placeTypes.myPlaceTypes[i].area[next].y());
			vert.push_back(Vertex(QVector3D(placeTypes.myPlaceTypes[i].area[next].x(), placeTypes.myPlaceTypes[i].area[next].y(), z + 2), color, QVector3D(), QVector3D()));

		}
		rendManager.addStaticGeometry("placetype", vert, "", GL_LINES, 1);
	}
}

/**
 * Block情報に基づいて、歩道などのモデルを生成する。
 * Blockのcontourには、歩道も含めたブロックの外郭ポリゴンが設定されているが、
 * この関数の中で、歩道を除くブロックの外郭ポリゴンに更新される。
 */
void VBOPm::generateBlockMesh(VBORenderManager& rendManager, BlockSet& blocks, PlaceTypesMainClass& placeTypesIn) {
	if (!initializedLC) {
		initLC();//init LC textures
	}

	rendManager.removeStaticGeometry("3d_sidewalk");
	rendManager.removeStaticGeometry("3d_block");
	rendManager.removeStaticGeometry("3d_parcel");

	for (int i = 0; i < blocks.size(); ++i) {
		blocks[i].adaptToTerrain(&rendManager);

		// Blockの3Dモデルを生成（Block表示モードの時にのみ、表示される）
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
				vert.push_back(Vertex(QVector3D(blocks[i].blockContour.contour[sN].x(),blocks[i].blockContour.contour[sN].y(), 1), color, QVector3D(), QVector3D()));
				vert.push_back(Vertex(QVector3D(blocks[i].blockContour.contour[next].x(),blocks[i].blockContour.contour[next].y(), 1), color, QVector3D(), QVector3D()));
			}
			rendManager.addStaticGeometry("3d_block",vert,"",GL_LINES,1);
		}
	
		// Parcelの3Dモデルを生成（Parcel表示モードの時にのみ、表示される）
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

					vert.push_back(Vertex(QVector3D(blocks[i].myParcels[*vi].parcelContour.contour[j].x(), blocks[i].myParcels[*vi].parcelContour.contour[j].y(), 1), color, QVector3D(), QVector3D()));
					vert.push_back(Vertex(QVector3D(blocks[i].myParcels[*vi].parcelContour.contour[next].x(), blocks[i].myParcels[*vi].parcelContour.contour[next].y(), 1), color, QVector3D(), QVector3D()));
				}
			
				rendManager.addStaticGeometry("3d_parcel", vert, "", GL_LINES, 1);
			}
		}
		
		// 歩道の3Dモデルを生成（通常の表示モードの時にのみ、表示される）
		{
			if (blocks[i].isPark==true) {
				// PARK
				int randPark=qrand()%grassFileNames.size();
				rendManager.addStaticGeometry2("3d_sidewalk",blocks[i].blockContour.contour,0.0f,false,grassFileNames[randPark],GL_QUADS,2,QVector3D(0.05f,0.05f,0.05f),QVector3D());
			} else {
				// NORMAL
				int randSidewalk=qrand()%sideWalkFileNames.size();

				rendManager.addStaticGeometry2("3d_sidewalk",blocks[i].blockContour.contour,0.5f,false,sideWalkFileNames[randSidewalk],GL_QUADS,2,sideWalkScale[randSidewalk],QVector3D());
				//sides
				std::vector<Vertex> vert;
				for(int sN=0;sN<blocks[i].blockContour.contour.size();sN++){
					int ind1 = sN;
					int ind2 = (sN+1) % blocks[i].blockContour.contour.size();
					QVector3D dir = blocks[i].blockContour.contour[ind2] - blocks[i].blockContour.contour[ind1];
					float length = dir.length();
					dir /= length;
					//printf("z %f\n",blocks[bN].blockContour.contour[ind1].z());
					QVector3D p1 = blocks[i].blockContour.contour[ind1]+QVector3D(0,0, 0.0f);//1.0f);
					QVector3D p2 = blocks[i].blockContour.contour[ind2]+QVector3D(0,0, 0.0f);//1.0f);
					QVector3D p3 = blocks[i].blockContour.contour[ind2]+QVector3D(0,0, 0.5f);//1.5f);
					QVector3D p4 = blocks[i].blockContour.contour[ind1]+QVector3D(0,0, 0.5f);//1.5f);
					QVector3D normal = QVector3D::crossProduct(p2-p1,p4-p1).normalized();
					vert.push_back(Vertex(p1,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p2,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p3,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
					vert.push_back(Vertex(p4,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
				}
				rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_QUADS,1|mode_Lighting);

				Block::parcelGraphVertexIter vi, viEnd;
			
				int randPark=qrand()%grassFileNames.size();
				for (boost::tie(vi, viEnd) = boost::vertices(blocks[i].myParcels); vi != viEnd; ++vi) {
					if (blocks[i].myParcels[*vi].parcelType == PAR_PARK) {
						//rendManager.addStaticGeometry2("3d_sidewalk", blocks[i].myParcels[*vi].parcelContour.contour,0.0f,false,grassFileNames[randPark],GL_QUADS,2,QVector3D(0.05f,0.05f,0.05f),QVector3D());
					}
				}
			}
		}

		// 歩道の分を確保するため、ブロックを縮小する。
		Loop3D blockContourInset;
		float sidewalk_width = placeTypesIn.myPlaceTypes[blocks[i].getMyPlaceTypeIdx()].getFloat("sidewalk_width");
		blocks[i].blockContour.computeInset(sidewalk_width, blockContourInset, false);
		blocks[i].blockContour.contour = blockContourInset;
		blocks[i].blockContour.getBBox3D(blocks[i].bbox.minPt, blocks[i].bbox.maxPt);
	}
}

void VBOPm::generatePopulationJobDistribution(BlockSet& blocks) {
	QVector2D cbd(1000, 1000);

	int rows = 100;
	int cols = 100;

	cv::Mat population = cv::Mat(rows, cols, CV_32F, cv::Scalar(0.0f));
	cv::Mat jobs = cv::Mat(rows, cols, CV_32F, cv::Scalar(0.0f));

	for (int i = 0; i < blocks.size(); ++i) {
		Block::parcelGraphVertexIter vi, viEnd;
		for (boost::tie(vi, viEnd) = boost::vertices(blocks[i].myParcels); vi != viEnd; ++vi) {
			if (blocks[i].myParcels[*vi].myBuilding.numStories <= 0) continue;

			// area
			boost::geometry::ring_type<Polygon3D>::type bg_footprint;
			boost::geometry::assign(bg_footprint, blocks[i].myParcels[*vi].myBuilding.buildingFootprint.contour);
			boost::geometry::correct(bg_footprint);
			float area = fabs(boost::geometry::area(bg_footprint));
			if (area <= 0) continue;

			// population
			float numPeople = area / 10.856f * blocks[i].myParcels[*vi].myBuilding.numStories;

			// position
			int c = (blocks[i].myParcels[*vi].bbox.midPt().x() + 5000.0f) / 100.0f;
			if (c < 0) c = 0;
			if (c >= cols) c = cols - 1;
			int r = (blocks[i].myParcels[*vi].bbox.midPt().y() + 5000.0f) / 100.0f;
			if (r < 0) r = 0;
			if (r >= rows) r = rows - 1;

			// residential / industrial ?
			float sigma2 = 0.2f;
			float dist_ratio = (cbd - blocks[i].myParcels[*vi].bbox.midPt()).length() / 5000.0f;
			float rand = expf(-dist_ratio * dist_ratio / 2.0f / sigma2);// / sqrtf(2.0f * M_PI * sigma2);
			if (rand > 1) rand = 1.0f;

			// industrial
			jobs.at<float>(r, c) += numPeople * rand;
			// residential
			population.at<float>(r, c) += numPeople * (1.0f - rand);
		}
	}

	HeatMapColorTable ct(0, 255);

	cv::Mat normalizedJobs(rows, cols, CV_8UC3, cv::Scalar(0, 0, 0));
	cv::Mat normalizedPopulation(rows, cols, CV_8UC3, cv::Scalar(0, 0, 0));
	for (int r = 0; r < jobs.rows; ++r) {
		for (int c = 0; c < jobs.cols; ++c) {
			int data= jobs.at<float>(r, c) * 255 / 1000;
			if (data > 255) data = 255;
			normalizedJobs.at<cv::Vec3b>(r, c) = cv::Vec3b(ct.getColor(data).blue(), ct.getColor(data).green(), ct.getColor(data).red());

			data = population.at<float>(r, c) * 255 / 1000;
			if (data > 255) data = 255;
			normalizedPopulation.at<cv::Vec3b>(r, c) = cv::Vec3b(ct.getColor(data).blue(), ct.getColor(data).green(), ct.getColor(data).red());
		}
	}

	cv::flip(normalizedJobs, normalizedJobs, 0);
	cv::flip(normalizedPopulation, normalizedPopulation, 0);
	cv::imwrite("jobs.png", normalizedJobs);
	cv::imwrite("population.png", normalizedPopulation);
}
