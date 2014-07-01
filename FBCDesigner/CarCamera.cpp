#include "CarCamera.h"
#include "Util.h"

void CarCamera::updatePerspective(int width,int height) {

	float aspect=(float)width/(float)height;
	float zfar=30000.0f;//90000.0f;
	float znear=0.1f;

	float f = 1.0f / tan (fovy * (0.00872664625f));//PI/360

	double m[16]=
	{	 f/aspect,	0,								0,									0,
				0,	f,								0,						 			0,
			    0,	0,		(zfar+znear)/(znear-zfar),		(2.0f*zfar*znear)/(znear-zfar),
			    0,	0,		    				   -1,									0

	};
	pMatrix=QMatrix4x4(m);
}

void CarCamera::updateCamMatrix() {
	float xrot = Util::rad2deg(atan2f(viewDir.z(), fabs(viewDir.y()))) - 90.0f;
	float yrot = 0.0f;
	float zrot = 90.0f - Util::rad2deg(atan2f(viewDir.y(), viewDir.x()));

	std::cout << pos.x() << "," << pos.y() << "," << pos.z() << " " << xrot << "," << zrot << std::endl;

	// modelview matrix
	mvMatrix.setToIdentity();
	mvMatrix.rotate(xrot, 1.0, 0.0, 0.0);		
	mvMatrix.rotate(yrot, 0.0, 1.0, 0.0);
	mvMatrix.rotate(zrot, 0.0, 0.0, 1.0);
	mvMatrix.translate(-pos.x(), -pos.y(), -pos.z());
	// normal matrix
	normalMatrix=mvMatrix.normalMatrix();
	// mvp
	mvpMatrix=pMatrix*mvMatrix;
}

void CarCamera::resetCamera() {
	pos.setX(0.0f);
	pos.setY(0.0f);
	pos.setZ(rendManager->getTerrainHeight(pos.x(), pos.y()) + carHeight);

	viewDir = QVector3D(0, 1, 0);
	up = QVector3D(0, 0, 1);
}

void CarCamera::moveForward(float speed) {
	float th0 = atan2f(viewDir.y(), viewDir.x());
	th0 += theta;
	float mag = sqrtf(viewDir.x() * viewDir.x() + viewDir.y() * viewDir.y());

	// change the view direction based on the steering direction
	viewDir = QVector3D(cosf(th0) * mag, sinf(th0) * mag, viewDir.z());
	viewDir.normalize();

	QVector3D newPos;
	newPos.setX(pos.x() + viewDir.x() * speed);
	newPos.setY(pos.y() + viewDir.y() * speed);
	newPos.setZ(rendManager->getTerrainHeight(pos.x(), pos.y()) + carHeight);

	// update the view direction according to the last movement
	viewDir = (newPos - pos).normalized();

	pos = newPos;

	// update the up direction
	QVector3D right = QVector3D::crossProduct(viewDir, QVector3D(0, 0, 1));
	up = QVector3D::crossProduct(right, viewDir);
	up.normalize();
}

void CarCamera::steer(float th) {
	theta = th;
}
