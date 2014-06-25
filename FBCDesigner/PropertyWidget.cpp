#include "PropertyWidget.h"
#include "MainWindow.h"
#include "GraphUtil.h"

PropertyWidget::PropertyWidget(MainWindow* mainWin) : QDockWidget("Property Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);

	// register the event handlers
	connect(ui.pushButtonVertexSearch, SIGNAL(clicked()), this, SLOT(searchVertex()));

	// register the event handlers
	hide();
}

void PropertyWidget::setRoadVertex(RoadGraph &roads, RoadVertexDesc vertexDesc, RoadVertexPtr selectedVertex) {
	QString desc("");
	QString location("");

	desc.setNum(vertexDesc);

	location = QString("(%1, %2)").arg(roads.graph[vertexDesc]->pt.x(), 0, 'f', 0).arg(roads.graph[vertexDesc]->pt.y(), 0, 'f', 0);

	QString onBoundary = roads.graph[vertexDesc]->onBoundary ? "Yes" : "No";

	QString neighbors;
	std::vector<RoadVertexDesc> n = GraphUtil::getNeighbors(roads, vertexDesc);
	for (int i = 0; i < n.size(); i++) {
		QString str;
		str.setNum(n[i]);

		neighbors += str;
		if (i < n.size() - 1) neighbors += ",";
	}

	QString deadend = selectedVertex->properties["deadend"].toBool() ? "Yes" : "No";

	ui.lineEditVertexDesc->setText(desc);
	ui.lineEditVertexPos->setText(location);
	ui.lineEditVertexOnBoundary->setText(onBoundary);
	ui.textEditVertexNeighbors->setText(neighbors);
	ui.lineEditVertexDeadend->setText(deadend);
}

/**
 * Display the selected edge information.
 */
void PropertyWidget::setRoadEdge(RoadGraph &roads, RoadEdgeDesc edgeDesc, RoadEdgePtr selectedEdge) {
	QString source;
	QString target;
	QString type;
	QString numLanes;
	QString oneWay;
	QString link;
	QString roundabout;

	if (selectedEdge != NULL) {
		RoadVertexDesc src = boost::source(edgeDesc, roads.graph);
		RoadVertexDesc tgt = boost::target(edgeDesc, roads.graph);
		source = QString("%1").arg(src);
		target = QString("%1").arg(tgt);

		switch (selectedEdge->type) {
		case RoadEdge::TYPE_HIGHWAY:
			type = "Highway";
			break;
		case RoadEdge::TYPE_AVENUE:
			type = "Avenue";
			break;
		case RoadEdge::TYPE_STREET:
			type = "Street";
			break;
		default:
			type = "";
			break;
		}

		numLanes.setNum(selectedEdge->lanes);

		oneWay = selectedEdge->oneWay ? "Yes" : "No";
		link = selectedEdge->link ? "Yes" : "No";
		roundabout = selectedEdge->roundabout ? "Yes" : "No";
	}

	ui.lineEditEdgeSource->setText(source);
	ui.lineEditEdgeTarget->setText(target);
	ui.lineEditEdgeType->setText(type);
	ui.lineEditEdgeLanes->setText(numLanes);
	ui.lineEditEdgeOneWay->setText(oneWay);
	ui.lineEditEdgeLink->setText(link);
	ui.lineEditEdgeRoundabout->setText(roundabout);
}

void PropertyWidget::setBlock(int id, Block& block) {
	QString str;
	str.setNum(id);
	ui.lineEditBlockId->setText(str);
	ui.lineEditBlockIsPark->setText(block.isPark ? "yes" : "no");
}

void PropertyWidget::resetRoadVertex() {
	ui.lineEditVertexDesc->setText("");
	ui.lineEditVertexPos->setText("");
	ui.lineEditVertexOnBoundary->setText("");
	ui.textEditVertexNeighbors->setText("");
	ui.lineEditVertexDeadend->setText("");
}

void PropertyWidget::resetRoadEdge() {
	ui.lineEditEdgeSource->setText("");
	ui.lineEditEdgeTarget->setText("");
	ui.lineEditEdgeType->setText("");
	ui.lineEditEdgeLanes->setText("");
	ui.lineEditEdgeOneWay->setText("");
	ui.lineEditEdgeLink->setText("");
	ui.lineEditEdgeRoundabout->setText("");
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void PropertyWidget::searchVertex() {
	RoadVertexDesc v_desc = ui.lineEditVertexSearch->text().toUInt();

	if (v_desc < boost::num_vertices(mainWin->urbanGeometry->roads.graph)) {
		mainWin->glWidget->selectVertex(mainWin->urbanGeometry->roads, v_desc);

		mainWin->glWidget->updateGL();
	}
}
