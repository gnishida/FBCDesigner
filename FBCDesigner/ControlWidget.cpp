#include "ControlWidget.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "UrbanGeometry.h"
#include "GLWidget3D.h"
#include "global.h"
#include "GraphUtil.h"
#include "BBox.h"

ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	connect(ui.terrainPaint_sizeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateTerrainLabels(int)));
	connect(ui.terrainPaint_changeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateTerrainLabels(int)));
	connect(ui.render_2DparksSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.terrain_smooth, SIGNAL(clicked()),this, SLOT(smoothTerrain()));
		
	updateRender2D(-1); // update just labels
	updateTerrainLabels(-1);
	
	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void ControlWidget::updateTerrainLabels(int newValue){
	int size=ui.terrainPaint_sizeSlider->value();
	ui.terrainPaint_sizeLabel->setText("Size: "+QString::number(size)+"%");
	G::global()["2DterrainEditSize"]=size/100.0f;

	float change=ui.terrainPaint_changeSlider->value()*1785/100.0f;
	ui.terrainPaint_changeLabel->setText("Ch: "+QString::number(change,'f',0)+"m");
	G::global()["2DterrainEditChange"]=change;
}//

void ControlWidget::updateRender2D(int newValue){
	int parkPer=ui.render_2DparksSlider->value();
	ui.render_2DparksLabel->setText("Park: "+QString::number(parkPer)+"%");
	G::global()["2d_parkPer"]=parkPer;
		
	if(newValue!=-1){//init
		mainWin->urbanGeometry->roads.modified=true;//force 
		mainWin->glWidget->updateGL();
	}
}//

void ControlWidget::smoothTerrain(){
	mainWin->glWidget->vboRenderManager.vboTerrain.smoothTerrain();
	mainWin->urbanGeometry->adaptToTerrain();
	mainWin->glWidget->updateGL();
}//

/*
void ControlWidget::camera3D() {
	mainWin->glWidget->camera=&mainWin->glWidget->camera3D;
	G::global()["rend_mode"]=1;
	update3D();

	mainWin->glWidget->updateCamera();
}
*/