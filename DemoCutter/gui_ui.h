
/********************************************************************************
** Form generated from reading UI file 'DemoCutterOA3096.ui'
**
** Created: Mon 18. Jul 22:17:05 2011
**      by: Qt User Interface Compiler version 4.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef GUI_UI_H
#define GUI_UI_H

#include "demo.h"
#include "cutter.h"

#pragma comment(lib, "shcore.lib")
#pragma comment(lib, "d3d9.lib")

#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>

#define VERSION "v0.9.12"

/*
 
v0.9.12 - integrated with Qt6/Conan/Cmake
v0.9.11 - relaxed checks to deal with delta from invalid frame
v0.9.10 - extented minutes limit from 999 to 9999
v0.9.9 - ia32 - thx hugo
v0.9.8 - fixed bug with truncated demos
v0.9.7 - updated demo libraries used
v0.9.6 - speed optimization
v0.9.5 - playerstate/pilotstate net field bug fix
v0.9.4 - several bugs fixed
v0.9.3 - Demo library std exceptions implemented
v0.9.2 - minor fix
v0.9.1 - added exception catching/printing
v0.9.0 - increased width of window
v0.8.10 - added valid range check
v0.8.9 - added map times range
v0.8.8 - added timer
v0.8.7 - added support for map restarts
v0.8.6 - fixed issues with cutting on later maps

TODO: - implement asserts in demo library, check every
        assumptions we do about demo format

*/

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget* centralwidget;
    QComboBox* beginMap;
    QComboBox* endMap;
    QLineEdit* beginTime;
    QLineEdit* endTime;
    QGroupBox* groupBoxStep2;
    QGroupBox* groupBoxStep1;
    QPushButton* inputFileButton;
    QLineEdit* inputFileName;
    QGroupBox* groupBoxStep3;
    QPushButton* outputFileButton;
    QLineEdit* outputFileName;
    QMenuBar* menubar;
    QStatusBar* statusbar;
    QLabel* labelBegin;
    QLabel* labelEnd;


    void setupUi(QMainWindow* MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->setFixedSize(400, 246);

        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));

        //Step1
        groupBoxStep1 = new QGroupBox(centralwidget);
        groupBoxStep1->setObjectName(QString::fromUtf8("groupBoxStep1"));
        groupBoxStep1->setGeometry(QRect(10, 10, 381, 51));

        inputFileButton = new QPushButton(groupBoxStep1);
        inputFileButton->setObjectName(QString::fromUtf8("inputFileButton"));
        inputFileButton->setGeometry(QRect(290, 25, 75, 21));

        inputFileName = new QLineEdit(groupBoxStep1);
        inputFileName->setObjectName(QString::fromUtf8("inputFileName"));
        inputFileName->setGeometry(QRect(10, 25, 271, 21));
        inputFileName->setReadOnly(true);

        //Step2
        groupBoxStep2 = new QGroupBox(centralwidget);
        groupBoxStep2->setObjectName(QString::fromUtf8("groupBoxStep2"));
        groupBoxStep2->setGeometry(QRect(10, 70, 381, 91));

        beginMap = new QComboBox(groupBoxStep2);
        beginMap->setObjectName(QString::fromUtf8("beginMap"));
        beginMap->setGeometry(QRect(44, 30, 237, 22));
        beginMap->setDisabled(true);
        beginMap->setInsertPolicy(QComboBox::InsertAtBottom);

        endMap = new QComboBox(groupBoxStep2);
        endMap->setObjectName(QString::fromUtf8("endMap"));
        endMap->setGeometry(QRect(44, 60, 237, 22));
        endMap->setDisabled(true);
        endMap->setInsertPolicy(QComboBox::InsertAtBottom);

        beginTime = new QLineEdit(groupBoxStep2);
        beginTime->setObjectName(QString::fromUtf8("beginTime"));
        beginTime->setGeometry(QRect(290, 30, 74, 21));
        beginTime->setDisabled(true);
        beginTime->setInputMask("0009:99.9;0");
        beginTime->setText("0:00.0");
        beginTime->setAlignment(Qt::AlignHCenter);

        endTime = new QLineEdit(groupBoxStep2);
        endTime->setObjectName(QString::fromUtf8("endTime"));
        endTime->setGeometry(QRect(290, 60, 74, 21));
        endTime->setDisabled(true);
        endTime->setInputMask("0009:99.9;0");
        endTime->setText("0:00.0");
        endTime->setAlignment(Qt::AlignHCenter);

        labelBegin = new QLabel(groupBoxStep2);
        labelBegin->setObjectName(QString::fromUtf8("labelBegin"));
        labelBegin->setGeometry(QRect(10, 30, 60, 21));
        labelEnd = new QLabel(groupBoxStep2);
        labelEnd->setObjectName(QString::fromUtf8("labelEnd"));
        labelEnd->setGeometry(QRect(10, 60, 60, 21));

        //Step3
        groupBoxStep3 = new QGroupBox(centralwidget);
        groupBoxStep3->setObjectName(QString::fromUtf8("groupBoxStep3"));
        groupBoxStep3->setGeometry(QRect(10, 170, 381, 51));

        outputFileButton = new QPushButton(groupBoxStep3);
        outputFileButton->setObjectName(QString::fromUtf8("outputFileButton"));
        outputFileButton->setGeometry(QRect(290, 25, 75, 21));
        outputFileButton->setDisabled(true);

        outputFileName = new QLineEdit(groupBoxStep3);
        outputFileName->setObjectName(QString::fromUtf8("outputFileName"));
        outputFileName->setGeometry(QRect(10, 25, 271, 21));
        outputFileName->setDisabled(true);
        outputFileName->setReadOnly(true);

        MainWindow->setCentralWidget(centralwidget);

        //menu bar
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 317, 21));
        MainWindow->setMenuBar(menubar);

        //status bar
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);

    } // setupUi

    void retranslateUi(QMainWindow* MainWindow) {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "JKA Demo Cutter " VERSION " by Sil", 0));
        groupBoxStep2->setTitle(QApplication::translate("MainWindow", "Step 2: Cutting Parameters", 0));
        groupBoxStep1->setTitle(QApplication::translate("MainWindow", "Step 1: Load input demo", 0));
        inputFileButton->setText(QApplication::translate("MainWindow", "Open", 0));
        groupBoxStep3->setTitle(QApplication::translate("MainWindow", "Step 3: Output", 0));
        outputFileButton->setText(QApplication::translate("MainWindow", "Cut!", 0));
        labelBegin->setText(QApplication::translate("MainWindow", "Begin:", 0));
        labelEnd->setText(QApplication::translate("MainWindow", "End:", 0));

    } // retranslateUi

};

namespace Ui {
    class MainWindow : public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // GUI_H