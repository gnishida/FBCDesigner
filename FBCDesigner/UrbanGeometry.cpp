#include "UrbanGeometry.h"
#include <limits>
#include <iostream>
#include <QFile>
#include "common.h"
#include "global.h"
#include "RendererHelper.h"
#include "GraphUtil.h"
#include "MainWindow.h"
#include "Util.h"
#include "VBOPm.h"
#include "VBOPmBlocks.h"
#include "VBOPmParcels.h"

UrbanGeometry::UrbanGeometry(MainWindow* mainWin) {
	this->mainWin = mainWin;

	/////////////////////////////////////////
	// INIT PLACETYPE
	placeTypes.myPlaceTypes.clear();
	//std::vector<PlaceType> *placeTypesPtr = &(placeTypes.myPlaceTypes);
	//placeTypesPtr->clear();

	PlaceType tmpPlaceType;
	//----- place type category ----
	tmpPlaceType["pt_category"]= 0;
	//----- roads -----
	tmpPlaceType["pt_radius"] = 600.0f;
	tmpPlaceType["pt_radius2"] = 600.0f;
	tmpPlaceType["pt_edges_curvature"] = 0;
	tmpPlaceType["pt_edges_irregularity"] =	0;
	tmpPlaceType["pt_edges_lengthU"] =	350.0f;
	tmpPlaceType["pt_edges_lengthV"] = 200.0f;
	tmpPlaceType["pt_edges_width"] =			20.0f;//!!!!!! UPDATE LC::misctools::Global::global()->arterial_edges_width;
	tmpPlaceType["pt_num_departing"] =	4;
	tmpPlaceType["pt_orientation"] = 0;
	tmpPlaceType["pt_loc_edges_curvature"] = 0;
	tmpPlaceType["pt_loc_edges_irregularity"] =	0;
	tmpPlaceType["pt_loc_edges_lengthU"] =		0.01f*50;
	tmpPlaceType["pt_loc_edges_lengthV"] =	0.01f*50;
	tmpPlaceType["pt_cur_edges_count"] = 0;
	//----- parcels -----
	tmpPlaceType["pt_parcel_area_mean"] = 5000.0f;//3600;
	tmpPlaceType["pt_parcel_area_deviation"] =	49;
	tmpPlaceType["pt_parcel_split_deviation"] =	0.19;
	tmpPlaceType["pt_park_percentage"] =0.0f;
	//----- buildings -----
	tmpPlaceType["pt_parcel_setback_front"] =15.0f;
	tmpPlaceType["pt_parcel_setback_sides"] = 2.0f;
	tmpPlaceType["pt_building_height_mean"] = 12;
	tmpPlaceType["pt_building_height_deviation"] =	90;
	tmpPlaceType["pt_building_max_frontage"] =0;
	tmpPlaceType["pt_parcel_setback_rear"] =0;
	tmpPlaceType["pt_building_max_depth"] =0;
	//-------------------
	tmpPlaceType["pt_pt"]= QVector3D(   0.0f,    0.0f, 0.0f);

	placeTypes.myPlaceTypes.push_back(tmpPlaceType);
	//placeTypesPtr->push_back(tmpPlaceType);

	G::global()["num_place_types"]=1;

}

UrbanGeometry::~UrbanGeometry() {
}

void UrbanGeometry::clear() {
	clearGeometry();
}

void UrbanGeometry::clearGeometry() {
	//if (&mainWin->glWidget->vboRenderManager != NULL) delete &mainWin->glWidget->vboRenderManager;

	roads.clear();
}

/*void UrbanGeometry::render(VBORenderManager& vboRenderManager) {
	// draw the road graph
	roads.generateMesh(vboRenderManager, "roads_lines", "roads_points");
	vboRenderManager.renderStaticGeometry("roads_lines");
	vboRenderManager.renderStaticGeometry("roads_points");
}*/

/**
 * Adapt all geometry objects to &mainWin->glWidget->vboRenderManager.
 */
void UrbanGeometry::adaptToTerrain() {
	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
}

/*void UrbanGeometry::newTerrain(int width, int depth, int cellLength) {
	clear();
}*/

/*void UrbanGeometry::loadTerrain(const QString &filename) {
	printf("NOT IMPLEMENTED YET\n");
}

void UrbanGeometry::saveTerrain(const QString &filename) {
	printf("NOT IMPLEMENTED YET\n");
}*/

void UrbanGeometry::loadRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	roads.clear();
	GraphUtil::loadRoads(roads, filename);

	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	roads.updateRoadGraph(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::saveRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		std::cerr << "The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	GraphUtil::saveRoads(roads, filename);
}

void UrbanGeometry::clearRoads() {
	roads.clear();
}

void UrbanGeometry::loadBlocks(const QString& filename) {
	blocks.load(filename);
	VBOPmBlocks::assignPlaceTypeToBlocks(placeTypes, blocks);
	//VBOPmParcels::assignPlaceTypeToParcels(placeTypes, blocks.blocks);
	VBOPm::generateBlockMesh(mainWin->glWidget->vboRenderManager, blocks);
	//blocks.generateMesh(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::saveBlocks(const QString& filename) {
	blocks.save(filename);
}
