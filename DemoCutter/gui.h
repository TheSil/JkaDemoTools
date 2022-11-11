
#ifndef GUI_H
#define GUI_H

//! [0]
#include "gui_ui.h"
//! [0]

//! [1]
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QMainWindow* parent = 0);

private slots:
    void on_inputFileButton_clicked();
    void on_outputFileButton_clicked();

private:
    Ui::MainWindow ui;

    DemoJKA::Demo	inputDemo;

    void setDisabledStep2(bool disabled);
    void setDisabledStep3(bool disabled);
    void clearStep2();
    void clearStep3();

    QString formatTime(int seconds);

};
//! [1]

#endif