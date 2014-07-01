#include "GLWidget3D.h"
#include "Util.h"
#include "GraphUtil.h"
#include "MainWindow.h"
#include <gl/GLU.h>
#include "RendererHelper.h"
#include "VBOPm.h"

GLWidget3D::GLWidget3D(MainWindow* mainWin) : QGLWidget(QGLFormat(QGL::SampleBuffers), (QWidget*)mainWin) {
	this->mainWin = mainWin;

	camera2D.resetCamera();
	flyCamera.resetCamera();
	camera3D.resetCamera();
	//carCamera.resetCamera();
	carCamera.rendManager = &vboRenderManager;
	camera = &camera2D;
	camera->type = Camera::TYPE_2D;
	//camera = &carCamera;
	//camera->type = Camera::TYPE_CAR;

	spaceRadius=30000.0;
	farPlaneToSpaceRadiusFactor=5.0f;//N 5.0f

	rotationSensitivity = 0.4f;
	zoomSensitivity = 10.0f;

	controlPressed=false;
	shiftPressed=false;
	altPressed=false;
	keyMPressed=false;

	camera2D.setRotation(0, 0, 0);
	camera2D.setTranslation(0, 0, G::getFloat("MAX_Z"));//6000);

	vertexSelected = false;
	edgeSelected = false;

	shadowEnabled=true;
}

QSize GLWidget3D::minimumSizeHint() const {
	return QSize(200, 200);
}

QSize GLWidget3D::sizeHint() const {
	return QSize(400, 400);
}

void GLWidget3D::mousePressEvent(QMouseEvent *event) {
	QVector2D pos;

	if (Qt::ControlModifier == event->modifiers()) {
		controlPressed = true;
	} else {
		controlPressed = false;
	}

	this->setFocus();

	lastPos = event->pos();
	mouseTo2D(event->x(), event->y(), pos);

	if (altPressed) {
		float xM, yM, radi, change;
		int selectedIndex;
		std::pair<int, int> selectedIndexPair;

		switch (mainWin->mode) {
		case MainWindow::MODE_DEFAULT: // terrain editing
			// normal Gaussian edition
			change=mainWin->controlWidget->ui.terrainPaint_changeSlider->value()*0.003f;
			radi=mainWin->controlWidget->ui.terrainPaint_sizeSlider->value()*0.01f;
			if (event->buttons() & Qt::RightButton) {
				change = -change;
			}
			//mainWin->urbanGeometry->vboRenderManager->addValue(pos.x(), pos.y(), change);
			xM=1.0f-(vboRenderManager.side/2.0f-pos.x())/vboRenderManager.side;
			yM=1.0f-(vboRenderManager.side/2.0f-pos.y())/vboRenderManager.side;
			vboRenderManager.vboTerrain.updateTerrain(xM,yM,change,radi);//rad,change);
			mainWin->urbanGeometry->adaptToTerrain();/// !! GEN did not have it here (enough in move?)
			shadow.makeShadowMap(this);
			updateGL();
			break;
		case MainWindow::MODE_BLOCK: // select a block
			if (event->buttons() & Qt::LeftButton) {
				selectedIndex = mainWin->urbanGeometry->blocks.selectBlock(pos);
				if (selectedIndex >= 0) {
					mainWin->propertyWidget->setBlock(selectedIndex, mainWin->urbanGeometry->blocks[selectedIndex]);
				}

				VBOPm::generateBlockMesh(mainWin->glWidget->vboRenderManager, mainWin->urbanGeometry->blocks);
				updateGL();
			}
			break;
		case MainWindow::MODE_PARCEL:
			if (event->buttons() & Qt::LeftButton) {
				selectedIndexPair = mainWin->urbanGeometry->blocks.selectParcel(pos);
				if (selectedIndexPair.first >= 0) {
					mainWin->propertyWidget->setParcel(selectedIndexPair.first, selectedIndexPair.second, mainWin->urbanGeometry->blocks[selectedIndexPair.first]);
				}
				
				VBOPm::generateBlockMesh(mainWin->glWidget->vboRenderManager, mainWin->urbanGeometry->blocks);
				updateGL();
			}

			break;
		}
	} else {
		if (event->buttons() & Qt::LeftButton) {
			if (shiftPressed) {	// select an edge
				if (GraphUtil::getEdge(mainWin->urbanGeometry->roads, pos, 30, selectedEdgeDesc)) {
					selectEdge(mainWin->urbanGeometry->roads, selectedEdgeDesc);
				} else {
					vertexSelected = false;
					edgeSelected = false;
				}
			} else {	// select a vertex
				if (GraphUtil::getVertex(mainWin->urbanGeometry->roads, pos, 30, selectedVertexDesc)) {
					selectVertex(mainWin->urbanGeometry->roads, selectedVertexDesc);
				} else {
					vertexSelected = false;
					edgeSelected = false;
				}
			}
		}
	}
}

void GLWidget3D::mouseReleaseEvent(QMouseEvent *event) {
	switch (mainWin->mode) {
	case MainWindow::MODE_DEFAULT:
		break;
	}

	updateGL();

	return;
}

void GLWidget3D::mouseMoveEvent(QMouseEvent *event) {
	QVector2D pos;
	mouseTo2D(event->x(), event->y(), pos);

	float dx = (float)(event->x() - lastPos.x());
	float dy = (float)(event->y() - lastPos.y());
	//float camElevation = camera->getCamElevation();

	vboRenderManager.mousePos3D=pos.toVector3D();

	if (altPressed) {	// editing
		if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::RightButton) || (event->buttons() & Qt::MiddleButton)) {
			float xM, yM, radi, change;
			switch (mainWin->mode) {
			case MainWindow::MODE_DEFAULT: // terrain editing
				// normal Gaussian edition
				change=mainWin->controlWidget->ui.terrainPaint_changeSlider->value()*0.003f;
				radi=mainWin->controlWidget->ui.terrainPaint_sizeSlider->value()*0.01f;
				change*=0.2f;//while moving, it is not necessary to change much
				if (event->buttons() & Qt::RightButton) {
					change = -change;
				}
				if (event->buttons() & Qt::MiddleButton) {
					change=FLT_MAX;//hack: flat terrain
				}
				//mainWin->urbanGeometry->vboRenderManager->addValue(pos.x(), pos.y(), change);
				xM=1.0f-(vboRenderManager.side/2.0f-pos.x())/vboRenderManager.side;
				yM=1.0f-(vboRenderManager.side/2.0f-pos.y())/vboRenderManager.side;
				vboRenderManager.vboTerrain.updateTerrain(xM,yM,change,radi);//rad,change);

				mainWin->urbanGeometry->adaptToTerrain();
				shadow.makeShadowMap(this);
				break;
			case MainWindow::MODE_BLOCK: // block editing
				break;
			}
		}
	} else if (event->buttons() & Qt::LeftButton) {	// Rotate
		if (camera->type == Camera::TYPE_2D) {
			camera2D.changeXRotation(rotationSensitivity * dy);
			camera2D.changeZRotation(-rotationSensitivity * dx);
		}
		updateCamera();
		lastPos = event->pos();
	} else if (event->buttons() & Qt::MidButton) {
		if (camera->type == Camera::TYPE_2D) {
			camera2D.changeXYZTranslation(-dx, dy, 0);
		}
		updateCamera();
		lastPos = event->pos();
	} else if (event->buttons() & Qt::RightButton) {	// Zoom
		if (camera->type == Camera::TYPE_2D) {
			camera2D.changeXYZTranslation(0, 0, -zoomSensitivity * dy);
		}
		updateCamera();
		lastPos = event->pos();
	}

	updateGL();
}

void GLWidget3D::initializeGL() {

	//qglClearColor(QColor(113, 112, 117));
	qglClearColor(QColor(0, 0, 0));

	//---- GLEW extensions ----
	GLenum err = glewInit();
	if (GLEW_OK != err){// Problem: glewInit failed, something is seriously wrong.
		qDebug() << "Error: " << glewGetErrorString(err);
	}
	qDebug() << "Status: Using GLEW " << glewGetString(GLEW_VERSION);
	if (glewIsSupported("GL_VERSION_4_2"))
		printf("Ready for OpenGL 4.2\n");
	else {
		printf("OpenGL 4.2 not supported\n");
		exit(1);
	}
	const GLubyte* text=
		glGetString(GL_VERSION);
	printf("VERSION: %s\n",text);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPointSize(10.0f);

	////////////////////////////////
	G::global()["3d_render_mode"]=0;
	// init hatch tex
	std::vector<QString> hatchFiles;
	for(int i=0;i<=6;i++){//5 hatch + perlin + water normals
		hatchFiles.push_back("../data/textures/LC/hatch/h"+QString::number(i)+"b.png");
	}
	for(int i=0;i<=0;i++){//1 win (3 channels)
		hatchFiles.push_back("../data/textures/LC/hatch/win"+QString::number(i)+"b.png");//win0b
	}
	vboRenderManager.loadArrayTexture("hatching_array",hatchFiles);

	///
	vboRenderManager.init();
	updateCamera();
	shadow.initShadow(vboRenderManager.program,this);
	glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);//SHADOW: Disable
	//glUniform1i(glGetUniformLocation(vboRenderManager.program, "terrainMode"),1);//FLAT
		
	shadow.makeShadowMap(this);

	carCamera.resetCamera();
}

void GLWidget3D::resizeGL(int width, int height) {
	updateCamera();
}

void GLWidget3D::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_TEXTURE_2D);
	
	// NOTE: camera transformation is not necessary here since the updateCamera updates the uniforms each time they are changed

	drawScene(0);		
}

void GLWidget3D::drawScene(int drawMode) {
	glLineWidth(10);
	
	if (mainWin->mode == MainWindow::MODE_PLACETYPE) {
		glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);
		vboRenderManager.renderStaticGeometry("placetype");
		vboRenderManager.renderStaticGeometry(QString("3d_roads"));	
	} else if (mainWin->mode == MainWindow::MODE_BLOCK) {
		glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);
		vboRenderManager.renderStaticGeometry(QString("3d_block"));
	} else if (mainWin->mode == MainWindow::MODE_PARCEL) {
		glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);
		vboRenderManager.renderStaticGeometry(QString("3d_parcel"));			
	} else {
		glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);

		vboRenderManager.renderStaticGeometry(QString("sky"));
		vboRenderManager.vboWater.render(vboRenderManager);

		glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 1);//SHADOW: Render Normal with Shadows

		vboRenderManager.vboTerrain.render(vboRenderManager);

		vboRenderManager.renderStaticGeometry(QString("3d_sidewalk"));
		vboRenderManager.renderStaticGeometry(QString("3d_building"));
		vboRenderManager.renderStaticGeometry(QString("3d_building_fac"));

		vboRenderManager.renderStaticGeometry(QString("3d_trees"));//hatch
		vboRenderManager.renderAllStreetElementName("tree");//LC
		vboRenderManager.renderAllStreetElementName("streetLamp");//LC

		vboRenderManager.renderStaticGeometry(QString("3d_roads"));			
		vboRenderManager.renderStaticGeometry(QString("3d_roads_inter"));//
		vboRenderManager.renderStaticGeometry(QString("3d_roads_interCom"));//


		// draw the selected vertex and edge
		if (vertexSelected) {
			RendererHelper::renderPoint(vboRenderManager, "selected_vertex", selectedVertex->pt, QColor(0, 0, 255), selectedVertex->pt3D.z() + 2.0f);
		}
		if (edgeSelected) {
			Polyline3D polyline(selectedEdge->polyline3D);
			for (int i = 0; i < polyline.size(); ++i) polyline[i].setZ(polyline[i].z() + 10.0f);
			RendererHelper::renderPolyline(vboRenderManager, "selected_edge_lines", "selected_edge_points", polyline, QColor(0, 0, 255));
		}

	}



	

	if (mainWin->mode == MainWindow::MODE_DEFAULT) {
		// SHADOWS
		if(drawMode==1){
			glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 2);// SHADOW: From light

			vboRenderManager.vboTerrain.render(vboRenderManager);

			vboRenderManager.renderStaticGeometry(QString("3d_building"));
			vboRenderManager.renderStaticGeometry(QString("3d_building_fac"));

			vboRenderManager.renderStaticGeometry(QString("3d_trees"));//hatch
			vboRenderManager.renderAllStreetElementName("tree");//LC
			vboRenderManager.renderAllStreetElementName("streetLamp");//LC
		}
	}
}

/**
 * select a vertex
 */
void GLWidget3D::selectVertex(RoadGraph &roads, RoadVertexDesc v_desc) {
	selectedVertex = roads.graph[v_desc];
	mainWin->propertyWidget->setRoadVertex(roads, v_desc, selectedVertex);
	mainWin->propertyWidget->resetRoadEdge();

	vertexSelected = true;
	edgeSelected = false;
}

void GLWidget3D::selectEdge(RoadGraph &roads, RoadEdgeDesc e_desc) {
	selectedEdge = roads.graph[e_desc];
	mainWin->propertyWidget->resetRoadVertex();
	mainWin->propertyWidget->setRoadEdge(roads, e_desc, selectedEdge);

	vertexSelected = false;
	edgeSelected = true;
}

void GLWidget3D::keyPressEvent( QKeyEvent *e ){
	shiftPressed=false;
	controlPressed=false;
	altPressed=false;
	keyMPressed=false;

	switch( e->key() ){
	case Qt::Key_Shift:
		shiftPressed=true;
		break;
	case Qt::Key_Control:
		controlPressed=true;
		break;
	case Qt::Key_Alt:
		altPressed=true;
		if (mainWin->mode == MainWindow::MODE_DEFAULT) {
			vboRenderManager.editionMode=true;
			updateGL();
			setMouseTracking(true);
		}
		break;
	case Qt::Key_Escape:
		updateGL();
		break;
	case Qt::Key_Delete:
		if (mainWin->mode == MainWindow::MODE_DEFAULT) {
			if (edgeSelected) {
				selectedEdge->valid = false;
				mainWin->urbanGeometry->adaptToTerrain();
				edgeSelected = false;
				updateGL();
			}
		} else if (mainWin->mode == MainWindow::MODE_BLOCK) {
			mainWin->urbanGeometry->blocks.removeSelectedBlock();
			VBOPm::generateBlockMesh(mainWin->glWidget->vboRenderManager, mainWin->urbanGeometry->blocks);
			//mainWin->urbanGeometry->blocks.generateMesh(vboRenderManager);
			updateGL();
		}
		break;
	case Qt::Key_R:
		printf("Reseting camera pose\n");
		camera->resetCamera();
		break;
	case Qt::Key_Up:
		if (camera->type == Camera::TYPE_CAR) {
			carCamera.moveForward(1);
			updateCamera();
			updateGL();
		}
		break;
	case Qt::Key_Down:
		if (camera->type == Camera::TYPE_CAR) {
			carCamera.moveForward(-1);
			updateCamera();
			updateGL();
		}
		break;
	case Qt::Key_Right:
		if (camera->type == Camera::TYPE_CAR) {
			carCamera.steer(-0.1f);
			carCamera.moveForward(1);
			updateCamera();
			updateGL();
		}
		break;
	case Qt::Key_Left:
		if (camera->type == Camera::TYPE_CAR) {
			carCamera.steer(0.1f);
			carCamera.moveForward(1);
			updateCamera();
			updateGL();
		}
		break;
	case Qt::Key_W:
		camera3D.moveKey(0);updateCamera();updateGL();
		break;
	case Qt::Key_S:
		camera3D.moveKey(1);updateCamera();updateGL();
		break;
	case Qt::Key_D:
		camera3D.moveKey(2);updateCamera();updateGL();
		break;
	case Qt::Key_A:
		camera3D.moveKey(3);updateCamera();updateGL();
		break;
	case Qt::Key_Q:
		camera3D.moveKey(4);updateCamera();updateGL();
		break;
	default:
		;
	}
}

void GLWidget3D::keyReleaseEvent(QKeyEvent* e) {
	if (e->isAutoRepeat()) {
		e->ignore();
		return;
	}
	switch (e->key()) {
	case Qt::Key_Shift:
		shiftPressed=false;
		break;
	case Qt::Key_Control:
		controlPressed=false;
		break;
	case Qt::Key_Alt:
		altPressed=false;
		if (mainWin->mode == MainWindow::MODE_DEFAULT) {
			vboRenderManager.editionMode=false;
			setMouseTracking(false);
			updateGL();
		}
	case Qt::Key_Right:
	case Qt::Key_Left:
		if (camera->type == Camera::TYPE_CAR) {
			carCamera.steer(0.0f);
			//updateCamera();
			//updateGL();
		}
		break;
	default:
		;
	}
}

/**
 * Convert the screen space coordinate (x, y) to the model space coordinate.
 */
void GLWidget3D::mouseTo2D(int x,int y, QVector2D &result) {
	updateCamera();
	updateGL();
	GLint viewport[4];

	// retrieve the matrices
	glGetIntegerv(GL_VIEWPORT, viewport);

	// retrieve the projected z-buffer of the origin
	GLfloat winX,winY,winZ;
	winX = (float)x;
	winY = (float)viewport[3] - (float)y;

	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	GLdouble wx2, wy2, wz2;  /*  returned world x, y, z coords  */
	gluUnProject( winX, winY, 0.0f, camera->mvMatrix.data(), camera->pMatrix.data(), viewport, &wx, &wy, &wz);
	gluUnProject( winX, winY, 1.0f, camera->mvMatrix.data(), camera->pMatrix.data(), viewport, &wx2, &wy2, &wz2);
	double f = wz / ( wz2 - wz );
	double x2d = wx - f * (wx2 - wx );
	double y2d = wy - f * (wy2 - wy );	
	result.setX(x2d);
	result.setY(y2d);
}

// this method should be called after any camera transformation (perspective or modelview)
// it will update viewport, perspective, view matrix, and update the uniforms
void GLWidget3D::updateCamera(){
	// update matrices
	int height = this->height() ? this->height() : 1;
	glViewport(0, 0, (GLint)this->width(), (GLint)this->height());
	camera->updatePerspective(this->width(),height);
	camera->updateCamMatrix();
	//if(G::global()["3d_render_mode"]==1) 		camera3D.updateCamMatrix();
	// update uniforms
	float mvpMatrixArray[16];
	float mvMatrixArray[16];

	for(int i=0;i<16;i++){
		mvpMatrixArray[i]=camera->mvpMatrix.data()[i];
		mvMatrixArray[i]=camera->mvMatrix.data()[i];	
	}
	float normMatrixArray[9];
	for(int i=0;i<9;i++){
		normMatrixArray[i]=camera->normalMatrix.data()[i];
	}

	//glUniformMatrix4fv(mvpMatrixLoc,  1, false, mvpMatrixArray);
	glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program, "mvpMatrix"),  1, false, mvpMatrixArray);
	glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program, "mvMatrix"),  1, false, mvMatrixArray);
	glUniformMatrix3fv(glGetUniformLocation(vboRenderManager.program, "normalMatrix"),  1, false, normMatrixArray);

	// light poss
	QVector3D light_dir=camera3D.light_dir.toVector3D();
	glUniform3f(glGetUniformLocation(vboRenderManager.program, "lightDir"),light_dir.x(),light_dir.y(),light_dir.z());
}//

/*void GLWidget3D::generate3DGeometry(bool justRoads){
	GraphUtil::cleanEdges(mainWin->urbanGeometry->roads);
	GraphUtil::clean(mainWin->urbanGeometry->roads);

	printf("generate3DGeometry\n");
	G::global()["3d_render_mode"]=1;//LC

	//1. update roadgraph geometry
	if(justRoads){//just roads a bit higher
		G::global()["3d_road_deltaZ"]=10.0f;
		mainWin->controlWidget->ui.render_3DtreesCheckBox->setChecked(false);//not trees
	}else{
		G::global()["3d_road_deltaZ"]=1.0f;
	}

	//VBORoadGraph::updateRoadGraph(vboRenderManager, mainWin->urbanGeometry->roads);
	//2. generate blocks, parcels and buildings and vegetation
	VBOPm::generateBlocks(vboRenderManager, mainWin->urbanGeometry->roads, mainWin->urbanGeometry->blocks, mainWin->urbanGeometry->placeTypes);

	shadow.makeShadowMap(this);

	printf("<<generate3DGeometry\n");
}*/