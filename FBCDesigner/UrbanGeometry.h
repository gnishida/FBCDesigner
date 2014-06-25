#pragma once

#include "glew.h"

#include "VBORenderManager.h"
#include "RoadGraph.h"
#include "BlockSet.h"
#include "VBOPlaceType.h"

class MainWindow;

class UrbanGeometry {
public:
	int width;
	int depth;
	MainWindow* mainWin;

	PlaceTypesMainClass placeTypes;
	RoadGraph roads;
	BlockSet blocks;

public:
	UrbanGeometry(MainWindow* mainWin);
	~UrbanGeometry();

	/** getter for width */
	int getWidth() { return width; }

	/** getter for depth */
	int getDepth() { return depth; }

	void clear();
	void clearGeometry();

	//void render(VBORenderManager &vboRenderManager);
	void adaptToTerrain();

	//void newTerrain(int width, int depth, int cellLength);
	//void loadTerrain(const QString &filename);
	//void saveTerrain(const QString &filename);

	void loadRoads(const QString& filename);
	void saveRoads(const QString& filename);
	void clearRoads();

	void loadBlocks(const QString& filename);
	void saveBlocks(const QString& filename);

private:
};
