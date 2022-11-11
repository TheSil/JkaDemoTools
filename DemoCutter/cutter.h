#ifndef CUTTER_H
#define CUTTER_H

#include "..\DemoManipulator\demo.h"

using namespace DemoJKA;

Snapshot* getFirstSnapshot(Message* message);
int getStartTime(Demo* demo, int mapIndex);
int getSnapshotTime(Message* message);

class Cutter {

public:
    static void	cutFromTime(Demo* demo, int time, int mapIndex);
    static void	cutToTime(Demo* demo, int time, int mapIndex);

};

#endif