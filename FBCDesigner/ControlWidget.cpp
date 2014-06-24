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
	connect(ui.terrain_2DShader, SIGNAL(stateChanged(int)),this, SLOT(changeTerrainShader(int)));
	connect(ui.render_2DroadsStrokeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DroadsExtraWidthSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DparksSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DparcelLineSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
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
		float stroke=ui.render_2DroadsStrokeSlider->value()*0.1f;
		ui.render_2DroadsStrokeLabel->setText("Stroke: "+QString::number(stroke,'f',1)+"");
		G::global()["2DroadsStroke"]=stroke;

		float extraWidth=ui.render_2DroadsExtraWidthSlider->value()*0.1f;
		ui.render_2DroadsExtraWidthLabel->setText("R Width: "+QString::number(extraWidth,'f',1)+"");
		G::global()["2DroadsExtraWidth"]=extraWidth;

		int parkPer=ui.render_2DparksSlider->value();
		ui.render_2DparksLabel->setText("Park: "+QString::number(parkPer)+"%");
		G::global()["2d_parkPer"]=parkPer;

		G::global()["maxBlockSizeForPark"] = ui.lineEditMaxBlockSizeForPark->text().toFloat();

		float parcelLine=ui.render_2DparcelLineSlider->value()*0.1f;
		ui.render_2DparcelLineLabel->setText("Par. Line: "+QString::number(parcelLine,'f',1)+"");
		G::global()["2d_parcelLine"]=parcelLine;

		if(newValue!=-1){//init
			mainWin->urbanGeometry->roads.modified=true;//force 
			mainWin->glWidget->updateGL();
		}
}//

void ControlWidget::changeTerrainShader(int){
	bool shader2D=ui.terrain_2DShader->isChecked();
	G::global()["shader2D"] = shader2D;
	int terrainMode;
	if(shader2D==true){
		terrainMode=0;
	}else 
		terrainMode=1;
	printf("terrainMode %d\n",terrainMode);
	mainWin->glWidget->vboRenderManager.changeTerrainShader(terrainMode);//could have used !shader2D
	mainWin->urbanGeometry->adaptToTerrain();
	mainWin->glWidget->updateGL();
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