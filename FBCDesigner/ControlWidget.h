#pragma once

#include <QDockWidget>
#include "ui_ControlWidget.h"

class MainWindow;

class ControlWidget : public QDockWidget {
Q_OBJECT

private:
	MainWindow* mainWin;

public:
	Ui::ControlWidget ui;
	ControlWidget(MainWindow* mainWin);

public slots:
	void updateTerrainLabels(int newValue);
	void smoothTerrain();
};

