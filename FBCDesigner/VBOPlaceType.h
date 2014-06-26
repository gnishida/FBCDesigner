/************************************************************************************************
*		Place Type Description
*		@author igarciad
************************************************************************************************/
#pragma once

#include <QVariant>
#include <QSettings>
#include <vector>
#include <qvariant.h>
#include <QDomNode>
#include <QTextStream>
#include "global.h"
#include "Polygon2D.h"

/**
* PlaceType class contains a place type instance in the city.
**/
class PlaceType {
public:
	Polygon2D area;
	QHash<QString, QVariant> attributes;

public:
	PlaceType() {}
	~PlaceType(void) {}

	void save(QDomDocument& doc, QDomNode& parent);
	void load(QDomNode& node);

	bool containsPoint (QVector2D &testPt);

	QVariant operator [](QString i) const    {return attributes[i];}
	QVariant & operator [](QString i) {return attributes[i];}

	QVector3D getQVector3D(QString i){
		if(!attributes.contains(i)){printf("PlaceType does not contain type %s\n",i.toAscii().constData());return QVector3D();}
		return attributes[i].value<QVector3D>();}
	float getFloat(QString i){
		if(!attributes.contains(i)){printf("PlaceType does not contain type %s\n",i.toAscii().constData());return 0;}
		return attributes[i].toFloat();}
	float getInt(QString i){
		if(!attributes.contains(i)){printf("PlaceType does not contain type %s\n",i.toAscii().constData());return 0;}
		return attributes[i].toInt();}
};

/**
* Main container class for place types
**/
class PlaceTypesMainClass {
public:
	std::vector<PlaceType> myPlaceTypes;

public:
	PlaceTypesMainClass() {}
	~PlaceTypesMainClass() {}

	size_t size() const { return myPlaceTypes.size(); }

	void save(const QString& filename);
	void load(const QString& filename );
};
