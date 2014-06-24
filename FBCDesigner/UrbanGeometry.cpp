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

UrbanGeometry::UrbanGeometry(MainWindow* mainWin) {
	this->mainWin = mainWin;

	//&mainWin->glWidget->vboRenderManager = NULL;

	//waterRenderer = new mylib::WaterRenderer(3000, 3000, -2.0f);

	//loadTerrain("../data/default.trn");

	//selectedAreaIndex = -1;
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

void UrbanGeometry::render(VBORenderManager& vboRenderManager) {
	// draw the road graph
	roads.generateMesh(vboRenderManager, "roads_lines", "roads_points");
	vboRenderManager.renderStaticGeometry("roads_lines");
	vboRenderManager.renderStaticGeometry("roads_points");
}

/**
 * Adapt all geometry objects to &mainWin->glWidget->vboRenderManager.
 */
void UrbanGeometry::adaptToTerrain() {
	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::newTerrain(int width, int depth, int cellLength) {
	clear();
}

void UrbanGeometry::loadTerrain(const QString &filename) {
	printf("NOT IMPLEMENTED YET\n");
	/*QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "MyUrbanGeometry::loadInfoLayers... The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	clear();

	QTextStream in(&file);
	QString line = in.readLine();
	this->width = line.split(" ")[0].toInt();
	this->depth = line.split(" ")[1].toInt();
	int cellLength = line.split(" ")[2].toInt();

	&mainWin->glWidget->vboRenderManager = new Terrain(width, depth, cellLength);

	for (int i = 0; i < &mainWin->glWidget->vboRenderManager->getNumRows() * &mainWin->glWidget->vboRenderManager->getNumCols(); ++i) {
		line = in.readLine();
		int idxX = line.split(" ").at(1).toInt();
		int idxY = line.split(" ").at(2).toInt();

		line = in.readLine();
		float x = line.split(" ").at(0).toFloat();
		float y = line.split(" ").at(1).toFloat();

		line = in.readLine();
		&mainWin->glWidget->vboRenderManager->setValue(x, y, line.toFloat());
	}
	*/

}

void UrbanGeometry::saveTerrain(const QString &filename) {
	printf("NOT IMPLEMENTED YET\n");
	/*QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		std::cerr << "MyUrbanGeometry::saveInfoLayers... The file is not writable: " << filename.toUtf8().constData() << endl;
		throw "The file is not writable: " + filename;
	}

	QTextStream out(&file);
	out << &mainWin->glWidget->vboRenderManager->width << " " << &mainWin->glWidget->vboRenderManager->depth << " " << &mainWin->glWidget->vboRenderManager->getCellLength() << endl;
	int count = 0;
	for (int i = 0; i < &mainWin->glWidget->vboRenderManager->getNumCols(); ++i) {
		for (int j = 0; j < &mainWin->glWidget->vboRenderManager->getNumRows(); ++j) {
			out << count++ << " " << i << " " << j << endl;
			float x = &mainWin->glWidget->vboRenderManager->getCell(i, j).getX();
			float y = &mainWin->glWidget->vboRenderManager->getCell(i, j).getY();

			out << x << " " << y << endl;

			out << &mainWin->glWidget->vboRenderManager->getTerrainHeight(x, y) << endl;
		}
	}*/
}

void UrbanGeometry::loadRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	roads.clear();
	GraphUtil::loadRoads(roads, filename);

	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
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
