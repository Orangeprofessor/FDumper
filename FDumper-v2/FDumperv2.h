#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_FDumperv2.h"

#include "ctpl_stl.hpp"

class FDumperv2 : public QMainWindow
{
	Q_OBJECT

public:
	FDumperv2(QWidget *parent = Q_NULLPTR);

private slots:
	virtual void OnAddQueueButton();

private:
	ctpl::thread_pool m_pool;
	Ui::FDumperv2Class m_ui;
};
