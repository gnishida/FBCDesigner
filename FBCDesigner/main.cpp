#include "MainWindow.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[]) {
	G::global()["parcelAreaMean"] = 3600;
	G::global()["parcelAreaDeviation"] = 49;
	G::global()["parcelSplitDeviation"] = 0.19;
	G::global()["parkPercentage"] = 0.2f;
	G::global()["parksRatio"] = 0.05f;

	G::global()["buildingHeightMean"] = 12;
	G::global()["buildingHeightDeviation"] = 90;
	G::global()["buildingBaseAboveGround"] = 3.0f;
	G::global()["meanBuildingFloorHeight"] = 3.0f;
	G::global()["buildingHeightFactor"] = 0.067f;
	G::global()["maxHeightBaseAspectRatio"] = 1.0f;
	G::global()["minBuildingArea"] = 10.0f;


	G::global()["MAX_Z"] = 4500.0f;

	G::global()["3d_road_deltaZ"]=1.0f;

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
