/************************************************************************************************
*		Place Type Description
*		@author igarciad
************************************************************************************************/

#include "VBOPlaceType.h"
#include "qstringlist.h"
#include <QFile>
#include <QTextStream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void PlaceType::save(QDomDocument& doc, QDomNode& parent) {
	QDomElement node = doc.createElement("placeType");

	for (QHash<QString, QVariant>::iterator it = attributes.begin(); it != attributes.end(); ++it) {
		QDomElement child = doc.createElement("attribute");
		child.setAttribute("name", it.key());
		child.setAttribute("type", it.value().typeName());
		child.setAttribute("value", it.value().toString());
		node.appendChild(child);
	}
	for (int i = 0; i < area.size(); ++i) {
		QDomElement child = doc.createElement("point");
		child.setAttribute("x", area[i].x());
		child.setAttribute("y", area[i].y());
		node.appendChild(child);
	}

	parent.appendChild(node);
}

void PlaceType::load(QDomNode& node) {
	area.clear();
	attributes.clear();

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "attribute") {
			if (child.toElement().attribute("type") == "float") {
				attributes[child.toElement().attribute("name")] = child.toElement().attribute("value").toFloat();
			}
		} else if (child.toElement().tagName() == "point") {
			QVector2D pt;
			pt.setX(child.toElement().attribute("x").toFloat());
			pt.setY(child.toElement().attribute("y").toFloat());
			area.push_back(pt);
		}

		child = child.nextSibling();
	}
}

//returns true if bounding rectangle contains testPt
bool PlaceType::containsPoint(QVector2D& testPt) {
	return area.contains(testPt);
}

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
void PlaceTypesMainClass::save(const QString& filename) {
	QFile file(filename);
     
    if (!file.open(QFile::WriteOnly| QFile::Truncate)) return;

	QDomDocument doc;
	QDomElement root = doc.createElement("blocks");
	doc.appendChild(root);

	for (int i = 0; i < myPlaceTypes.size(); ++i) {		
		myPlaceTypes[i].save(doc, root);
	}

	QTextStream out(&file);
	doc.save(out, 4);
}

void PlaceTypesMainClass::load(const QString& filename) {
	myPlaceTypes.clear();

	QFile file(filename);

	QDomDocument doc;
	doc.setContent(&file, true);
	QDomElement root = doc.documentElement();

	QDomNode node = root.firstChild();
	while (!node.isNull()) {
		if (node.toElement().tagName() == "placeType") {
			PlaceType placeType;

			placeType.load(node);

			myPlaceTypes.push_back(placeType);
		}

		node = node.nextSibling();
	}
}

