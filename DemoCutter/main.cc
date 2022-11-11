#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "demo.h"
#include "gui.h"
#include <time.h>
#include <sstream>
#include <iomanip>

#include <QtWidgets\QApplication>
#include <QtCore\QFile>
#include <QApplication>
#include <QStyleFactory>

using namespace DemoJKA;
using namespace std;

bool CheckArguments(int argCount) {
    if (argCount != 7)
        return false;

    return true;
}

int main(int argc, char** argv) {
    DemoJKA::MessageBuffer::initHuffman();
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    app.setStyle(QStyleFactory::create("Fusion"));
    return app.exec();
}
