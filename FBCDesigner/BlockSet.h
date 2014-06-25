#pragma once

#include "VBORenderManager.h"
#include <vector>
#include <QString>
#include <QDomNode>
#include <QVector2D>
#include "VBOBlock.h"
#include "VBOParcel.h"

class BlockSet {
public:
	std::vector<Block> blocks;
	int selectedIndex;
	bool modified;

public:
	BlockSet() : selectedIndex(-1), modified(false) {}

	void setModified() { modified = true; }
	void load(const QString& filename);
	void save(const QString& filename);
	void generateMesh(VBORenderManager& rendManager);

	int select(const QVector2D& pos);
	void removeSelectedBlock();

	Block& operator[](int index) { return blocks[index]; }
	const Block& operator[](int index) const { return blocks[index]; }
	size_t size() const { return blocks.size(); }

private:
	void loadBlock(QDomNode& node, Block& block);
	void saveBlock(QDomDocument& doc, QDomNode& node, Block& block);
	void loadParcel(QDomNode& node, Block& block);
	void saveParcel(QDomDocument& doc, QDomNode& node, Parcel& parcel);

};

