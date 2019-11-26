#include "stdafx.h"
#include "FDumperv2.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	FDumperv2 w;
	w.show();
	return a.exec();
}
