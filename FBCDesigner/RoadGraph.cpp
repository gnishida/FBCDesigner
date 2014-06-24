#include "RoadGraph.h"
#include <QGLWidget>
#include "GraphUtil.h"
#include "Util.h"
#include "global.h"

/*
#include "../render/Terrain.h"
#include "../render/RenderableCircleList.h"
#include "../render/RenderableCylinderList.h"
#include "../render/TextureManager.h"
*/

RoadGraph::RoadGraph() {
	showHighways = true;
	showBoulevards = true;
	showAvenues = true;
	showLocalStreets = true;

	renderMode = RENDER_DEFAULT;
	//renderMode = RENDER_GROUPBY;

	modified = false;
}

RoadGraph::~RoadGraph() {
}

void RoadGraph::generateMesh(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN) {
	if (!modified) return;

	modified = false;

	renderManager.removeStaticGeometry(linesN);
	renderManager.removeStaticGeometry(pointsN);

	_generateMeshVerticesDefault(renderManager, linesN, pointsN);
}

void RoadGraph::_generateMeshVerticesDefault(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN) {

	//////////////////////////////////////
	// EDGES
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
			if (!graph[*ei]->valid) continue;

			int num = graph[*ei]->polyline3D.size();
			if (num <= 1) continue;

			float halfWidth = graph[*ei]->getWidth()*0.5f;//it should not have /2.0f (but compensated below)
			

			std::vector<Vertex> vert(4*(num - 1));
			std::vector<Vertex> vertBg(4*(num - 1));
			
			// Type
			QColor color;// = graph[*ei]->color;
			QColor colorO;
			float heightOffset = 0.0f;
			float heightOffsetO=0.0f;
			bool render = false;

			switch (graph[*ei]->type) {
			case RoadEdge::TYPE_HIGHWAY:
				if (showHighways) {
					render = true;
					heightOffset = 0.8f;
					heightOffsetO = 0.3f;
					color=QColor(0xfa,0x9e,0x25);
					colorO=QColor(0x00, 0x00, 0x00);//QColor(0xdf,0x9c,0x13);
					halfWidth*=1.4f;
				}
				break;
			case RoadEdge::TYPE_BOULEVARD:
				if (showBoulevards) {
					render = true;
					heightOffset = 0.5f;
					heightOffsetO = 0.2f;
					color=QColor(0xff,0xe1,0x68);
					colorO=QColor(0x00, 0x00, 0x00);//QColor(0xe5,0xbd,0x4d);
					halfWidth*=1.4f;
				}
				break;
			case RoadEdge::TYPE_AVENUE:
				if (showAvenues) {
					render = true;
					heightOffset = 0.6f;
					heightOffsetO = 0.1f;
					color=QColor(0xff,0xe1,0x68);
					colorO=QColor(0x00, 0x00, 0x00);//QColor(0xe5,0xbd,0x4d);
					halfWidth*=1.4f;
				}
				break;
			case RoadEdge::TYPE_STREET:
				if (showLocalStreets) {
					render = true;
					heightOffset = 0.4f;
					heightOffsetO = 0.1f;
					color=QColor(0xff,0xff,0xff);
					colorO=QColor(0x00, 0x00, 0x00);//QColor(0xd7,0xd1,0xc7);
					halfWidth*=1.8f;
				}
				break;
			}

			if(render==false)
				continue;
			halfWidth+= G::global().getFloat("2DroadsExtraWidth");
			heightOffset+=0.45f;//to have park below
			heightOffsetO+=0.45f;//to have park below

			float halfWidthBg = halfWidth + G::global().getFloat("2DroadsStroke");//it should not depend on the type 3.5f

			QVector3D p0, p1, p2, p3;
			QVector3D p0Bg, p1Bg, p2Bg, p3Bg;
			for (int i = 0; i < num - 1; ++i) {
				QVector3D pt1 = graph[*ei]->polyline3D[i];
				QVector3D pt2 = graph[*ei]->polyline3D[i + 1];

				QVector3D perp = pt2 - pt1;
				perp = QVector3D(-perp.y(), perp.x(), 0.0f);
				perp.normalize();

				if (i == 0) {
					p0 = pt1 + perp * halfWidth;
					p1 = pt1 - perp * halfWidth;
					p0Bg = pt1 + perp * halfWidthBg;
					p1Bg = pt1 - perp * halfWidthBg;
				}
				p2 = pt2 - perp * halfWidth;
				p3 = pt2 + perp * halfWidth;
				p2Bg = pt2 - perp * halfWidthBg;
				p3Bg = pt2 + perp * halfWidthBg;
				QVector3D normal = Util::calculateNormal(p0, p1, p2);

				if (i < num - 2) {
					QVector3D pt3 = graph[*ei]->polyline3D[i + 2];

					Util::getIrregularBisector(pt1, pt2, pt3, halfWidth, halfWidth, p3);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidth, -halfWidth, p2);
					Util::getIrregularBisector(pt1, pt2, pt3, halfWidthBg, halfWidthBg, p3Bg);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidthBg, -halfWidthBg, p2Bg);
				}

					vert[i*4+0]=Vertex(p0.x(),p0.y(),p0.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+1]=Vertex(p1.x(),p1.y(),p1.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+2]=Vertex(p2.x(),p2.y(),p2.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+3]=Vertex(p3.x(),p3.y(),p3.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					
					vertBg[i*4+0]=Vertex(p0Bg.x(),p0Bg.y(),p0Bg.z()+heightOffsetO,colorO.redF(),colorO.greenF(),colorO.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vertBg[i*4+1]=Vertex(p1Bg.x(),p1Bg.y(),p1Bg.z()+heightOffsetO,colorO.redF(),colorO.greenF(),colorO.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vertBg[i*4+2]=Vertex(p2Bg.x(),p2Bg.y(),p2Bg.z()+heightOffsetO,colorO.redF(),colorO.greenF(),colorO.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vertBg[i*4+3]=Vertex(p3Bg.x(),p3Bg.y(),p3Bg.z()+heightOffsetO,colorO.redF(),colorO.greenF(),colorO.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					

				p0 = p3;
				p1 = p2;
				p0Bg = p3Bg;
				p1Bg = p2Bg;
			}

			renderManager.addStaticGeometry(linesN, vert, "", GL_QUADS, 1);//MODE=1 color
			renderManager.addStaticGeometry(linesN, vertBg, "", GL_QUADS, 1);//MODE=1 color
		}
	}

	/////////////////////////////////////////////////////
	// INTERSECTIONS
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
			if (!graph[*vi]->valid) continue;

			// get the largest width of the outing edges
			QColor color;// = graph[*ei]->color;
			QColor colorO;
			float heightOffset = 0.0f;
			float heightOffsetO=0.0f;
			int maxType=-1;
			float halfWidth;

			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, graph); oei != oeend; ++oei) {
				if (!graph[*oei]->valid) continue;
				//printf("type %d\n",graph[*oei]->type);
				if(maxType>graph[*oei]->type)
					continue;
				maxType=graph[*oei]->type;
				halfWidth=graph[*oei]->getWidth()*0.5f;//it should not have /2.0f (but compensated below)

				switch (graph[*oei]->type) {
				case RoadEdge::TYPE_HIGHWAY:
						heightOffset = 0.6f;
						heightOffsetO = 0.3f;
						color=QColor(0xfa,0x9e,0x25);
						colorO=QColor(0x00, 0x00, 0x00);//QColor(0xdf,0x9c,0x13);
						halfWidth*=1.4f;
					continue;
				case RoadEdge::TYPE_BOULEVARD:
						heightOffset = 0.5f;
						heightOffsetO = 0.2f;
						color=QColor(0xff,0xe1,0x68);
						colorO=QColor(0x00, 0x00, 0x00);//QColor(0xe5,0xbd,0x4d);
						halfWidth*=1.4f;
					continue;
				case RoadEdge::TYPE_AVENUE:
						heightOffset = 0.5f;
						heightOffsetO = 0.2f;
						color=QColor(0xff,0xe1,0x68);
						colorO=QColor(0x00, 0x00, 0x00);//QColor(0xe5,0xbd,0x4d);
						halfWidth*=1.4f;
					continue;
				case RoadEdge::TYPE_STREET:
						heightOffset = 0.4f;
						heightOffsetO = 0.2f;
						color=QColor(0xff,0xff,0xff);
						colorO=QColor(0x00, 0x00, 0x00);//QColor(0xd7,0xd1,0xc7);
						halfWidth*=1.8f;
					continue;
				}
			}

			halfWidth+= G::global().getFloat("2DroadsExtraWidth");
			heightOffset+=0.45f;//to have park below
			heightOffsetO+=0.45f;//to have park below

			float max_r=halfWidth;
			float max_rO=halfWidth + G::global().getFloat("2DroadsStroke");//it should not depend on the type 3.5f

			std::vector<Vertex> vert(3*20);
			std::vector<Vertex> vertBg(3*20);

			for (int i = 0; i < 20; ++i) {
				float angle1 = 2.0 * M_PI * i / 20.0f;
				float angle2 = 2.0 * M_PI * (i + 1) / 20.0f;

				vert[i*3+0]=Vertex(graph[*vi]->pt3D.x(), graph[*vi]->pt3D.y(), graph[*vi]->pt3D.z() + heightOffset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+1]=Vertex(graph[*vi]->pt3D.x() + max_r * cosf(angle1), graph[*vi]->pt3D.y() + max_r * sinf(angle1), graph[*vi]->pt3D.z() + heightOffset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+2]=Vertex(graph[*vi]->pt3D.x() + max_r * cosf(angle2), graph[*vi]->pt3D.y() + max_r * sinf(angle2), graph[*vi]->pt3D.z() + heightOffset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);

				vertBg[i*3+0]=Vertex(graph[*vi]->pt3D.x(), graph[*vi]->pt3D.y(), graph[*vi]->pt3D.z() + heightOffsetO, colorO.redF(), colorO.greenF(), colorO.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vertBg[i*3+1]=Vertex(graph[*vi]->pt3D.x() + max_rO * cosf(angle1), graph[*vi]->pt3D.y() + max_rO * sinf(angle1), graph[*vi]->pt3D.z() + heightOffsetO, colorO.redF(), colorO.greenF(), colorO.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vertBg[i*3+2]=Vertex(graph[*vi]->pt3D.x() + max_rO * cosf(angle2), graph[*vi]->pt3D.y() + max_rO * sinf(angle2), graph[*vi]->pt3D.z() + heightOffsetO, colorO.redF(), colorO.greenF(), colorO.blueF(), 0, 0, 1.0f, 0, 0, 0);
			}
			
			renderManager.addStaticGeometry(pointsN, vert, "", GL_TRIANGLES, 1);//MODE=1 color
			renderManager.addStaticGeometry(pointsN, vertBg, "", GL_TRIANGLES, 1);//MODE=1 color
		}
	}

	return;

	// draw edges
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
			if (!graph[*ei]->valid) continue;

			int num = graph[*ei]->polyline3D.size();
			if (num <= 1) continue;

			float halfWidth = graph[*ei]->getWidth() / 2.0f;
			float halfWidthBg = halfWidth * 1.2f;

			std::vector<Vertex> vert(4*(num - 1));
			QColor color = graph[*ei]->color;

			QVector3D p0, p1, p2, p3;
			QVector3D p0Bg, p1Bg, p2Bg, p3Bg;
			for (int i = 0; i < num - 1; ++i) {
				QVector3D pt1 = graph[*ei]->polyline3D[i];
				QVector3D pt2 = graph[*ei]->polyline3D[i + 1];

				QVector3D vec = pt2 - pt1;
				vec = QVector3D(-vec.y(), vec.x(), 0.0f);
				vec.normalize();

				if (i == 0) {
					p0 = pt1 + vec * halfWidth;
					p1 = pt1 - vec * halfWidth;
					p0Bg = pt1 + vec * halfWidthBg;
					p1Bg = pt1 - vec * halfWidthBg;
				}
				p2 = pt2 - vec * halfWidth;
				p3 = pt2 + vec * halfWidth;
				p2Bg = pt2 - vec * halfWidthBg;
				p3Bg = pt2 + vec * halfWidthBg;
				QVector3D normal = Util::calculateNormal(p0, p1, p2);

				if (i < num - 2) {
					QVector3D pt3 = graph[*ei]->polyline3D[i + 2];
					
					Util::getIrregularBisector(pt1, pt2, pt3, halfWidth, halfWidth, p3);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidth, -halfWidth, p2);
					Util::getIrregularBisector(pt1, pt2, pt3, halfWidthBg, halfWidthBg, p3Bg);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidthBg, -halfWidthBg, p2Bg);
				}

				float heightOffset = 0.0f;
				bool render = false;

				switch (graph[*ei]->type) {
				case RoadEdge::TYPE_HIGHWAY:
					if (showHighways) {
						render = true;
						heightOffset = 0.7f;
					}
					break;
				case RoadEdge::TYPE_BOULEVARD:
					if (showBoulevards) {
						render = true;
						heightOffset = 0.5f;
					}
					break;
				case RoadEdge::TYPE_AVENUE:
					if (showAvenues) {
						render = true;
						heightOffset = 0.5f;
					}
					break;
				case RoadEdge::TYPE_STREET:
					if (showLocalStreets) {
						render = true;
						heightOffset = 0.3f;
					}
					break;
				}

				if (render) {
					vert[i*4+0]=Vertex(p0.x(),p0.y(),p0.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+1]=Vertex(p1.x(),p1.y(),p1.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+2]=Vertex(p2.x(),p2.y(),p2.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+3]=Vertex(p3.x(),p3.y(),p3.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
				}

				p0 = p3;
				p1 = p2;
				p0Bg = p3Bg;
				p1Bg = p2Bg;
			}

			renderManager.addStaticGeometry(linesN, vert, "", GL_QUADS, 1);//MODE=1 color
		}
	}

	// draw intersections
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
			if (!graph[*vi]->valid) continue;

			// get the largest width of the outing edges
			float max_r = 0;
			int max_roadType = 0;
			QColor color, bgColor;
			float offset = 0.3f;
			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, graph); oei != oeend; ++oei) {
				if (!graph[*oei]->valid) continue;

				float r = graph[*oei]->getWidth() * 0.5f;
				if (r > max_r) {
					max_r = r;
				}

				if (graph[*oei]->type > max_roadType) {
					max_roadType = graph[*oei]->type;
					color = graph[*oei]->color;
					bgColor = graph[*oei]->bgColor;
				}
			}

			float max_rBg = max_r * 1.2f;

			if (max_roadType == RoadEdge::TYPE_AVENUE) {
				offset = 0.5f;
			}

			std::vector<Vertex> vert(3*20);

			for (int i = 0; i < 20; ++i) {
				float angle1 = 2.0 * M_PI * i / 20.0f;
				float angle2 = 2.0 * M_PI * (i + 1) / 20.0f;

				vert[i*3+0]=Vertex(graph[*vi]->pt3D.x(), graph[*vi]->pt3D.y(), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+1]=Vertex(graph[*vi]->pt3D.x() + max_r * cosf(angle1), graph[*vi]->pt3D.y() + max_r * sinf(angle1), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+2]=Vertex(graph[*vi]->pt3D.x() + max_r * cosf(angle2), graph[*vi]->pt3D.y() + max_r * sinf(angle2), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
			}

			renderManager.addStaticGeometry(pointsN, vert, "", GL_TRIANGLES, 1);//MODE=1 color
		}
	}
}

void RoadGraph::clear() {
	graph.clear();

	modified = true;
}

/**
 * adapt this road graph to the vboRenderManager.
 */
void RoadGraph::adaptToTerrain(VBORenderManager* vboRenderManager) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
		float z = vboRenderManager->getTerrainHeight(graph[*vi]->pt.x(), graph[*vi]->pt.y(), true);
		graph[*vi]->pt3D = QVector3D(graph[*vi]->pt.x(), graph[*vi]->pt.y(), z + 2);
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, graph);
		RoadVertexDesc tgt = boost::target(*ei, graph);
		graph[*ei]->polyline3D.clear();

		Polyline2D polyline = GraphUtil::finerEdge(graph[*ei]->polyline, 10.0f);
		for (int i = 0; i < polyline.size(); ++i) {
			float z = vboRenderManager->getTerrainHeight(polyline[i].x(), polyline[i].y(), true);
			graph[*ei]->polyline3D.push_back(QVector3D(polyline[i].x(), polyline[i].y(), z + 2));
		}
	}

	setModified();
}

