#include "MainWindow.h"
#include <QFileDialog>
#include <QShortcut>
#include "TerrainSizeInputDialog.h"
#include "GraphUtil.h"
#include "Util.h"
#include "VBOPm.h"

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	// setup the docking widgets
	controlWidget = new ControlWidget(this);
	propertyWidget = new PropertyWidget(this);

	// setup the toolbar
	ui.fileToolBar->addAction(ui.actionLoadRoads);
	ui.fileToolBar->addAction(ui.actionSaveRoads);
	ui.modeToolBar->addAction(ui.actionModeDefault);
	ui.modeToolBar->addAction(ui.actionModeBlock);
	ui.modeToolBar->addAction(ui.actionModeParcel);

	ui.actionModeDefault->setChecked(true);

	// register the menu's action handlers
	connect(ui.actionNewTerrain, SIGNAL(triggered()), this, SLOT(onNewTerrain()));
	connect(ui.actionOpenTerrain, SIGNAL(triggered()), this, SLOT(onLoadTerrain()));
	connect(ui.actionSaveTerrain, SIGNAL(triggered()), this, SLOT(onSaveTerrain()));
	connect(ui.actionLoadPlaceTypes, SIGNAL(triggered()), this, SLOT(onLoadPlaceTypes()));
	connect(ui.actionLoadRoads, SIGNAL(triggered()), this, SLOT(onLoadRoads()));
	connect(ui.actionSaveRoads, SIGNAL(triggered()), this, SLOT(onSaveRoads()));
	connect(ui.actionClearRoads, SIGNAL(triggered()), this, SLOT(onClearRoads()));
	connect(ui.actionLoadBlocks, SIGNAL(triggered()), this, SLOT(onLoadBlocks()));
	connect(ui.actionSaveBlocks, SIGNAL(triggered()), this, SLOT(onSaveBlocks()));
	connect(ui.actionSaveRoads, SIGNAL(triggered()), this, SLOT(onSaveRoads()));
	connect(ui.actionSaveImage, SIGNAL(triggered()), this, SLOT(onSaveImage()));
	connect(ui.actionSaveImageHD, SIGNAL(triggered()), this, SLOT(onSaveImageHD()));
	connect(ui.actionLoadCamera, SIGNAL(triggered()), this, SLOT(onLoadCamera()));
	connect(ui.actionSaveCamera, SIGNAL(triggered()), this, SLOT(onSaveCamera()));
	connect(ui.actionResetCamera, SIGNAL(triggered()), this, SLOT(onResetCamera()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	connect(ui.actionModeDefault, SIGNAL(triggered()), this, SLOT(onModeDefault()));
	connect(ui.actionModePlaceType, SIGNAL(triggered()), this, SLOT(onModePlaceType()));
	connect(ui.actionModeBlock, SIGNAL(triggered()), this, SLOT(onModeBlock()));
	connect(ui.actionModeParcel, SIGNAL(triggered()), this, SLOT(onModeParcel()));

	connect(ui.actionGenerateBlocks, SIGNAL(triggered()), this, SLOT(onGenerateBlocks()));
	connect(ui.actionGenerateParcels, SIGNAL(triggered()), this, SLOT(onGenerateParcels()));
	connect(ui.actionGenerateBuildings, SIGNAL(triggered()), this, SLOT(onGenerateBuildings()));
	connect(ui.actionGenerateVegetation, SIGNAL(triggered()), this, SLOT(onGenerateVegetation()));
	connect(ui.actionGenerateAll, SIGNAL(triggered()), this, SLOT(onGenerateAll()));
	connect(ui.actionControlWidget, SIGNAL(triggered()), this, SLOT(onShowControlWidget()));
	connect(ui.actionPropertyWidget, SIGNAL(triggered()), this, SLOT(onShowPropertyWidget()));

	// setup the GL widget
	glWidget = new GLWidget3D(this);
	setCentralWidget(glWidget);

	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);

	urbanGeometry = new UrbanGeometry(this);

	mode = MODE_DEFAULT;
}

MainWindow::~MainWindow() {
}

void MainWindow::keyPressEvent(QKeyEvent* e) {
	glWidget->keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent* e) {
	glWidget->keyReleaseEvent(e);
}

void MainWindow::onNewTerrain() {
	TerrainSizeInputDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted) {
		//urbanGeometry->newTerrain(dlg.width, dlg.depth, dlg.cellLength);
		glWidget->vboRenderManager.changeTerrainDimensions(dlg.side,dlg.cellResolution);
		glWidget->updateGL();
	}
}

void MainWindow::onLoadTerrain() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Load Terrain file..."), "", tr("Terrain Files (*.png *.jpg)"));
	if (filename.isEmpty()) return;
	glWidget->vboRenderManager.vboTerrain.loadTerrain(filename);

	glWidget->updateGL();
}

void MainWindow::onSaveTerrain() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Terrain file..."), "", tr("Terrain Files (*.png)"));
	if (filename.isEmpty()) return;
	glWidget->vboRenderManager.vboTerrain.saveTerrain(filename);
}

void MainWindow::onLoadPlaceTypes() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Place Types file..."), "", tr("Place Types Files (*.xml)"));
	if (filename.isEmpty()) return;

	urbanGeometry->placeTypes.load(filename);

	glWidget->updateGL();
}

void MainWindow::onLoadRoads() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Street Map file..."), "", tr("StreetMap Files (*.gsm)"));
	if (filename.isEmpty()) return;

	urbanGeometry->loadRoads(filename);
	glWidget->shadow.makeShadowMap(glWidget);

	glWidget->updateGL();
}

void MainWindow::onSaveRoads() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Street Map file..."), "", tr("StreetMap Files (*.gsm)"));
	if (filename.isEmpty()) return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	urbanGeometry->saveRoads(filename);

	QApplication::restoreOverrideCursor();
}

void MainWindow::onClearRoads() {
	urbanGeometry->clearRoads();
	glWidget->updateGL();
}

void MainWindow::onLoadBlocks() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Block file..."), "", tr("Block Files (*.xml)"));
	if (filename.isEmpty()) return;

	urbanGeometry->loadBlocks(filename);
	urbanGeometry->adaptToTerrain();
	glWidget->updateGL();
}

void MainWindow::onSaveBlocks() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Block file..."), "", tr("Block Files (*.xml)"));
	if (filename.isEmpty()) return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	urbanGeometry->saveBlocks(filename);

	QApplication::restoreOverrideCursor();
}

void MainWindow::onSaveImage() {
	if(QDir("screenshots").exists()==false) QDir().mkdir("screenshots");
	QString fileName="screenshots/"+QDate::currentDate().toString("yyMMdd")+"_"+QTime::currentTime().toString("HHmmss")+".png";
	glWidget->grabFrameBuffer().save(fileName);
	printf("Save %s\n",fileName.toAscii().constData());
}//

void MainWindow::onSaveImageHD() {
	if(QDir("screenshots").exists()==false) QDir().mkdir("screenshots");
	QString fileName="screenshots/"+QDate::currentDate().toString("yyMMdd")+"_"+QTime::currentTime().toString("HHmmss")+".png";
	glWidget->grabFrameBuffer().save(fileName);

	QString fileNameHD="screenshots/"+QDate::currentDate().toString("yyMMdd")+"_"+QTime::currentTime().toString("HHmmss")+"_HD.png";
	int cH=glWidget->height();
	int cW=glWidget->width();
	glWidget->resize(cW*3,cH*3);
	glWidget->updateGL();
	glWidget->grabFrameBuffer().save(fileNameHD);
	glWidget->resize(cW,cH);
	glWidget->updateGL();

	printf("Save HD %s\n",fileName.toAscii().constData());
}//

void MainWindow::onLoadCamera() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Camera file..."), "", tr("Area Files (*.cam)"));
	if (filename.isEmpty()) return;

	glWidget->camera->loadCameraPose(filename);
	glWidget->updateCamera();

	glWidget->updateGL();
}

void MainWindow::onSaveCamera() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Camera file..."), "", tr("Area Files (*.cam)"));
	if (filename.isEmpty()) return;
	
	glWidget->camera->saveCameraPose(filename);
}

void MainWindow::onResetCamera() {
	glWidget->camera->resetCamera();
	glWidget->updateCamera();
	glWidget->updateGL();
}

void MainWindow::onModeDefault() {
	mode = MODE_DEFAULT;
	ui.actionModePlaceType->setChecked(false);
	ui.actionModeBlock->setChecked(false);
	ui.actionModeParcel->setChecked(false);

	VBOPm::generateBlockMesh(glWidget->vboRenderManager, urbanGeometry->blocks);
	glWidget->updateGL();
}

void MainWindow::onModePlaceType() {
	mode = MODE_PLACETYPE;
	ui.actionModeDefault->setChecked(false);
	ui.actionModeBlock->setChecked(false);
	ui.actionModeParcel->setChecked(false);

	VBOPm::generatePlaceTypeMesh(glWidget->vboRenderManager, urbanGeometry->placeTypes);
	glWidget->updateGL();
}

void MainWindow::onModeBlock() {
	mode = MODE_BLOCK;
	ui.actionModeDefault->setChecked(false);
	ui.actionModePlaceType->setChecked(false);
	ui.actionModeParcel->setChecked(false);

	VBOPm::generateBlockMesh(glWidget->vboRenderManager, urbanGeometry->blocks);
	glWidget->updateGL();
}

void MainWindow::onModeParcel() {
	mode = MODE_PARCEL;
	ui.actionModeDefault->setChecked(false);
	ui.actionModeBlock->setChecked(false);
	ui.actionModePlaceType->setChecked(false);

	VBOPm::generateBlockMesh(glWidget->vboRenderManager, urbanGeometry->blocks);
	glWidget->updateGL();
}

void MainWindow::onGenerateBlocks() {
	VBOPm::generateBlocks(glWidget->vboRenderManager, urbanGeometry->roads, urbanGeometry->blocks, urbanGeometry->placeTypes);
	VBOPm::generateBlockMesh(glWidget->vboRenderManager, urbanGeometry->blocks);
	glWidget->updateGL();
}

void MainWindow::onGenerateParcels() {
	VBOPm::generateParcels(glWidget->vboRenderManager, urbanGeometry->blocks, urbanGeometry->placeTypes);
	VBOPm::generateBlockMesh(glWidget->vboRenderManager, urbanGeometry->blocks);
	glWidget->updateGL();
}

void MainWindow::onGenerateBuildings() {
	VBOPm::generateBuildings(glWidget->vboRenderManager, urbanGeometry->blocks, urbanGeometry->placeTypes);
	glWidget->shadow.makeShadowMap(glWidget);
	glWidget->updateGL();
}

void MainWindow::onGenerateVegetation() {
	VBOPm::generateVegetation(glWidget->vboRenderManager, urbanGeometry->blocks, urbanGeometry->placeTypes);
	glWidget->shadow.makeShadowMap(glWidget);
	glWidget->updateGL();
}

void MainWindow::onGenerateAll() {
	VBOPm::generateBlocks(glWidget->vboRenderManager, urbanGeometry->roads, urbanGeometry->blocks, urbanGeometry->placeTypes);
	VBOPm::generateParcels(glWidget->vboRenderManager, urbanGeometry->blocks, urbanGeometry->placeTypes);
	VBOPm::generateBlockMesh(glWidget->vboRenderManager, urbanGeometry->blocks);
	VBOPm::generateBuildings(glWidget->vboRenderManager, urbanGeometry->blocks, urbanGeometry->placeTypes);
	VBOPm::generateVegetation(glWidget->vboRenderManager, urbanGeometry->blocks, urbanGeometry->placeTypes);
	glWidget->shadow.makeShadowMap(glWidget);

	glWidget->updateGL();
}

void MainWindow::onShowControlWidget() {
	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);
}

void MainWindow::onShowPropertyWidget() {
	propertyWidget->show();
	addDockWidget(Qt::RightDockWidgetArea, propertyWidget);
}
