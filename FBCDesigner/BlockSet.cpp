#include "BlockSet.h"
#include <QFile>
#include <QTextStream>
#include "Polygon2D.h"

void BlockSet::load(const QString& filename) {
	blocks.clear();

	QFile file(filename);

	QDomDocument doc;
	doc.setContent(&file, true);
	QDomElement root = doc.documentElement();

	QDomNode node = root.firstChild();
	while (!node.isNull()) {
		if (node.toElement().tagName() == "block") {
			Block block;

			loadBlock(node, block);

			blocks.push_back(block);
		}

		node = node.nextSibling();
	}

	modified = true;
}

void BlockSet::save(const QString& filename) {
	QFile file(filename);
     
    if (!file.open(QFile::WriteOnly| QFile::Truncate)) return;

	QDomDocument doc;
	QDomElement root = doc.createElement("blocks");
	doc.appendChild(root);

	for (int i = 0; i < blocks.size(); ++i) {
		QDomElement node = doc.createElement("block");
		node.setAttribute("id", i);
		node.setAttribute("isPark", blocks[i].isPark ? "yes" : "no");
		node.setAttribute("placeTypeIdx", blocks[i].getMyPlaceTypeIdx());

		saveBlock(doc, node, blocks[i]);

		root.appendChild(node);
	}

	QTextStream out(&file);
	doc.save(out, 4);
}

void BlockSet::loadBlock(QDomNode& node, Block& block) {
	block.isPark = node.toElement().attribute("isPark") == "yes" ? true : false;
	block.setMyPlaceTypeIdx(node.toElement().attribute("placeTypeIdx").toInt());

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "point") {
			QVector3D pt;
			pt.setX(child.toElement().attribute("x").toFloat());
			pt.setY(child.toElement().attribute("y").toFloat());
			pt.setZ(child.toElement().attribute("z").toFloat());
			block.blockContour.push_back(pt);			
		} else if (child.toElement().tagName() == "parcel") {
			loadParcel(child, block);
		}

		child = child.nextSibling();
	}
}

void BlockSet::saveBlock(QDomDocument& doc, QDomNode& node, Block& block) {
	//QDomElement node = doc.createElement("block");
	//node.setAttribute("isPark", block.isPark ? "yes" : "no");

	for (int i = 0; i < block.blockContour.contour.size(); ++i) {
		QDomElement child = doc.createElement("point");
		child.setAttribute("x", block.blockContour[i].x());
		child.setAttribute("y", block.blockContour[i].y());
		child.setAttribute("z", block.blockContour[i].z());
		node.appendChild(child);
	}

	if (block.isPark) return;

	Block::parcelGraphVertexIter vi, viEnd;
	int id = 0;
	for (boost::tie(vi, viEnd) = boost::vertices(block.myParcels); vi != viEnd; ++vi) {
		QDomElement child = doc.createElement("parcel");
		child.setAttribute("id", id++);

		saveParcel(doc, child, block.myParcels[*vi]);

		node.appendChild(child);
	}

	//parent.appendChild(node);
}

void BlockSet::loadParcel(QDomNode& node, Block& block) {
	Parcel parcel;
	Polygon3D polygon;

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "point") {
			QVector3D pt;
			pt.setX(child.toElement().attribute("x").toFloat());
			pt.setY(child.toElement().attribute("y").toFloat());
			pt.setZ(child.toElement().attribute("z").toFloat());
			polygon.push_back(pt);
			//parcel.parcelContour.push_back(pt);			
		}

		child = child.nextSibling();
	}

	parcel.setContour(polygon);

	Block::parcelGraphVertexDesc v_desc = boost::add_vertex(block.myParcels);
	block.myParcels[v_desc] = parcel;
}

void BlockSet::saveParcel(QDomDocument& doc, QDomNode& node, Parcel& parcel) {
	for (int i = 0; i < parcel.parcelContour.contour.size(); ++i) {
		QDomElement child = doc.createElement("point");
		child.setAttribute("x", parcel.parcelContour[i].x());
		child.setAttribute("y", parcel.parcelContour[i].y());
		child.setAttribute("z", parcel.parcelContour[i].z());
		node.appendChild(child);
	}
}

void BlockSet::generateMesh(VBORenderManager& rendManager) {
	if (!modified) return;

	modified = false;

	rendManager.removeStaticGeometry("3d_sidewalk");
	rendManager.removeStaticGeometry("3d_parcel");

	/*
	for(int bN=0;bN<blocks.size();bN++){
		std::vector<Vertex> vert;

		QVector3D color(0.6f, 0.6f, 0.6f);
		if (bN == selectedIndex) {
			color = QVector3D(1.0f, 1.0f, 1.0f);
		}

		for(int sN=0;sN<blocks[bN].blockContour.contour.size();sN++){
			int next = (sN+1) % blocks[bN].blockContour.contour.size();
			vert.push_back(Vertex(QVector3D(blocks[bN].blockContour.contour[sN].x(),blocks[bN].blockContour.contour[sN].y(),5),
				color,QVector3D(),QVector3D()));
			vert.push_back(Vertex(QVector3D(blocks[bN].blockContour.contour[next].x(),blocks[bN].blockContour.contour[next].y(),5),
				color,QVector3D(),QVector3D()));
		}
		//rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_POINTS,1|mode_AdaptTerrain);
		rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_LINES,1|mode_AdaptTerrain);
	}
	*/

	for (int bN=0;bN<blocks.size();bN++){
		//blocks[bN].generateMesh(rendManager);
	}

}

int BlockSet::selectBlock(const QVector2D& pos) {
	for (int i = 0; i < blocks.size(); ++i) {
		Polygon2D polygon;
		for (int j = 0; j < blocks[i].blockContour.contour.size(); ++j) {
			polygon.push_back(QVector2D(blocks[i].blockContour[j]));
		}

		if (polygon.contains(pos)) {
			if (selectedBlockIndex != i) {
				selectedBlockIndex = i;
				modified = true;
			}

			return i;
		}
	}

	if (selectedBlockIndex != -1) {
		selectedBlockIndex = -1;
		modified = true;
	}

	return -1;
}

std::pair<int, int> BlockSet::selectParcel(const QVector2D& pos) {
	for (int i = 0; i < blocks.size(); ++i) {
		Block::parcelGraphVertexIter vi, viEnd;
		int id = 0;
		for (boost::tie(vi, viEnd) = boost::vertices(blocks[i].myParcels); vi != viEnd; ++vi, ++id) {
			Polygon2D polygon;
			for (int j = 0; j < blocks[i].myParcels[*vi].parcelContour.contour.size(); ++j) {
				polygon.push_back(QVector2D(blocks[i].myParcels[*vi].parcelContour[j]));
			}

			if (polygon.contains(pos)) {
				if (selectedBlockIndex != i && selectedParcelIndex != id) {
					selectedBlockIndex = i;
					selectedParcelIndex = id;
					modified = true;
				}

				return std::make_pair(selectedBlockIndex, selectedParcelIndex);
			}
		}
	}

	if (selectedBlockIndex != -1) {
		selectedBlockIndex = -1;
		selectedParcelIndex = -1;
		modified = true;
	}

	return std::make_pair(-1, -1);
}

void BlockSet::removeSelectedBlock() {
	if (selectedBlockIndex < 0 || selectedBlockIndex >= blocks.size()) return;

	blocks.erase(blocks.begin() + selectedBlockIndex);

	selectedBlockIndex = -1;

	modified = true;
}