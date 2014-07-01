#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "glew.h"
#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"
#include "ControlWidget.h"
#include "PropertyWidget.h"
#include "GLWidget3D.h"
#include "UrbanGeometry.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	static enum { MODE_DEFAULT = 0, MODE_PLACETYPE, MODE_BLOCK, MODE_PARCEL };

public:
	Ui::MainWindow ui;
	ControlWidget* controlWidget;
	PropertyWidget* propertyWidget;
	GLWidget3D* glWidget;
	UrbanGeometry* urbanGeometry;

	int mode;

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

protected:
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

public slots:
	void onNewTerrain();
	void onLoadTerrain();
	void onSaveTerrain();
	void onLoadPlaceTypes();
	void onLoadRoads();
	void onSaveRoads();
	void onClearRoads();
	void onLoadBlocks();
	void onSaveBlocks();
	void onSaveImage();
	void onSaveImageHD();
	void onLoadCamera();
	void onSaveCamera();
	void onResetCamera();

	void onModeDefault();
	void onModePlaceType();
	void onModeBlock();
	void onModeParcel();

	void onGenerateBlocks();
	void onGenerateParcels();
	void onGenerateBuildings();
	void onGenerateVegetation();
	void onGenerateAll();

	void onShowControlWidget();
	void onShowPropertyWidget();

	void onCameraDefault();
	void onCameraCar();
};

#endif // MAINWINDOW_H
