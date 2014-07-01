#pragma once

#include "Camera.h"
#include "VBORenderManager.h"

class CarCamera : public Camera {
public:
	VBORenderManager* rendManager;

	float carHeight;
	float theta;
	QVector3D pos;
	QVector3D viewDir;
	QVector3D up;

public:
	CarCamera() : carHeight(2.0f), theta(0.0f) { fovy = 60.0f; }

	void updatePerspective(int width,int height);
	void updateCamMatrix();
	void resetCamera();
	void moveForward(float speed);
	void steer(float th);
};

