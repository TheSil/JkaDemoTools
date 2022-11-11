#include "gui.h"
#include <time.h>

MainWindow::MainWindow(QMainWindow *parent)
	     : QMainWindow(parent)
{
    ui.setupUi(this);
	setAcceptDrops(true);
}

void MainWindow::setDisabledStep2(bool disabled){
	ui.beginMap->setDisabled(disabled);
	ui.beginTime->setDisabled(disabled);
	ui.endMap->setDisabled(disabled);
	ui.endTime->setDisabled(disabled);
}

void MainWindow::setDisabledStep3(bool disabled){
	ui.outputFileButton->setDisabled(disabled);
	ui.outputFileName->setDisabled(disabled);
}

void MainWindow::clearStep2(){
	ui.outputFileName->setText("");
}

void MainWindow::clearStep3(){
	ui.beginMap->clear();
	ui.endMap->clear();

	ui.beginTime->setText("0:00.0");
	ui.endTime->setText("0:00.0");
}

QString MainWindow::formatTime(int seconds){
	int minutes = seconds/60;
	seconds -= minutes*60;

	QString ret = QString::number(minutes)+":";

	if (seconds < 10)
		ret += "0"+QString::number(seconds);
	else
		ret += QString::number(seconds); 

	return ret;
}

void MainWindow::on_inputFileButton_clicked(){
	QString s = QFileDialog::getOpenFileName(0, QString(), QString(),"*.dm_26");

	if (s.isEmpty())
		return;

	ui.inputFileName->setText(s);
	clearStep3();

	//cant change parameters after doing cutting, because demo is messed up
	//lets implement this later
	setDisabledStep2(true);
	setDisabledStep3(true);

	try{
		clock_t realTimeBegin, realTimeEnd;
		realTimeBegin = clock();

		//open demo
		ui.statusbar->showMessage("Loading demo...");
		inputDemo.open(s.toLatin1());

		//analyse to find map starts
		ui.statusbar->showMessage("Analysing...");
		inputDemo.analyse();

		realTimeEnd = clock();
		ui.statusbar->showMessage("Loaded. ("+QString::number(((float)(realTimeEnd-realTimeBegin)/CLOCKS_PER_SEC))+"s)");

	} catch (std::exception& e){
		ui.statusbar->showMessage("ERROR: "+QString(e.what()));
		return;
	}

	//ui.statusbar->clearMessage();

	if (inputDemo.getMapsCount() == 0)
		ui.statusbar->showMessage("Failed to load map list.");

	for(int i=0;i<inputDemo.getMapsCount();++i)	{
		QString mapName = (inputDemo.getMapName(i)).c_str();
		QString s = QString::number(i)+
			": "+
			mapName+
			" ("+
			formatTime(inputDemo.getMapStartTime(i))+
			" - "+
			formatTime(inputDemo.getMapEndTime(i))+
			")";

		ui.beginMap->addItem(s);
		ui.endMap->addItem(s);
	}

	clearStep2();
	setDisabledStep2(false);

	//disable step3 - in case someone changed input file
	setDisabledStep3(false);
}


void MainWindow::on_outputFileButton_clicked(){
	int beginMapIndex = ui.beginMap->currentIndex();
	int endMapIndex = ui.endMap->currentIndex();

	if (endMapIndex < beginMapIndex){
		ui.statusbar->showMessage("Error: Maps are not in chronological order.",3000);
		return;
	}

	//extract time
	int beginTimeMs, endTimeMs;

	int index, minutes;
	float seconds;
	QString tmpStr;


	tmpStr = ui.beginTime->displayText();
	index = tmpStr.indexOf(':');
	minutes = (tmpStr.left(index)).toInt();
	seconds = (tmpStr.right(tmpStr.size()-index-1)).toFloat();

	beginTimeMs = 1000*60*minutes + 1000*seconds;

	tmpStr = ui.endTime->displayText();
	index = tmpStr.indexOf(':');
	minutes = (tmpStr.left(index)).toInt();
	seconds = (tmpStr.right(tmpStr.size()-index-1)).toFloat();

	endTimeMs = 1000*60*minutes + 1000*seconds;

	if ((endMapIndex == beginMapIndex) && 
		(endTimeMs != 0) && 
		(beginTimeMs != 0) &&
		(endTimeMs < beginTimeMs)){
		ui.statusbar->showMessage("Error: Times are not in chronological order.",3000);
		return;
	}

	//range check
	if (beginTimeMs && 
		((beginTimeMs < inputDemo.getMapStartTime(beginMapIndex)*1000)
		|| (beginTimeMs > inputDemo.getMapEndTime(beginMapIndex)*1000))){
		ui.statusbar->showMessage("Error: Begin time is not in valid range.",3000);
		return;
	}

	if (endTimeMs && 
		((endTimeMs > inputDemo.getMapEndTime(endMapIndex)*1000)
		|| (endTimeMs < inputDemo.getMapStartTime(endMapIndex)*1000))){
		ui.statusbar->showMessage("Error: End time is not in valid range.",3000);
		return;
	}

	//parameters checks are done, ask for output destination
	QString s = QFileDialog::getSaveFileName(0,QString(),QString(),"*.dm_26");

    if (!s.endsWith(".dm_26"))
    {
        s += ".dm_26";       
    }

	if (s.isEmpty())
		return;

	//cant change parameters after doing cutting, because demo is messed up
	//lets implement this later
	setDisabledStep2(true);
	setDisabledStep3(true);

	ui.outputFileName->setText(s);

	clock_t realTimeBegin, realTimeEnd;
	realTimeBegin = clock();

	try{
		//lets cut off end
		ui.statusbar->showMessage("Cutting off end...");
		Cutter::cutToTime(&inputDemo, endTimeMs, endMapIndex);

		//and now cut off beginning
		ui.statusbar->showMessage("Cutting off beginning...");
		Cutter::cutFromTime(&inputDemo, beginTimeMs, beginMapIndex);

		ui.statusbar->showMessage("Saving Output Demo...");
		inputDemo.save(ui.outputFileName->text().toLatin1());

	} catch (std::exception& e){
		ui.statusbar->showMessage("ERROR: "+QString(e.what()));
		return;
	}

	realTimeEnd = clock();

	ui.statusbar->showMessage("Done! ("+QString::number(((float)(realTimeEnd-realTimeBegin)/CLOCKS_PER_SEC))+"s)");

}
