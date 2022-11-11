#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "..\DemoManipulator\demo.h"
#include "gui.h"
#include <time.h>
#include <sstream>
#include <iomanip>

#include <QtWidgets\QApplication>
#include <QtCore\QFile>

using namespace DemoJKA;
using namespace std;

//ofstream logfile("output.log");

bool CheckArguments(int argCount){
	if (argCount != 7)
		return false;

	return true;
}

int main(int argc, char** argv){

	DemoJKA::MessageBuffer::initHuffman();
    QApplication app(argc, argv);
	MainWindow mainWindow;
	mainWindow.show();
    return app.exec();

}