#include "stdafx.h"
#include "FDumperv2.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	auto fdumper = FDumperv2::getInstance();
	fdumper->show();
	return a.exec();
}
