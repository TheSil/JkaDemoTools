#include "cutter.h"

#pragma comment(lib, "dwrite")

Snapshot* getFirstSnapshot(Message* message) {

    for (int i = 0; i < message->getInstructionsCount(); ++i)
        if (message->getInstruction(i)->getType() == INSTR_SNAPSHOT)
            return message->getInstruction(i)->getSnapshot();

    return 0;
}

int getStartTime(Demo* demo, int mapIndex) {
    Message* firstMessage = demo->getMessage(demo->getMapId(mapIndex));
    if (!firstMessage)
        return -1;

    int ret;

    if (demo->isMapRestart(mapIndex)) {
        //ok map restart, we should find new map time in server command
        ServerCommand* command;
        std::string str;
        for (int i = 0; i < firstMessage->getInstructionsCount(); ++i) {
            if (command = firstMessage->getInstruction(i)->getServerCommand()) {
                str = command->getCommand();
                if (str.substr(0, 6) == "cs 21 ") {
                    std::stringstream stream(str.substr(7, str.size() - 7 - 2));
                    if ((stream >> ret)) {
                        return ret;
                    }
                }
            }
        }

        throw DemoException("restart time extraction failed");
    }
    else { //new map begins
        // we find new time directly in gamestate
        //configstring 21
        Gamestate* gamestate;
        for (int i = 0; i < firstMessage->getInstructionsCount(); ++i) {
            gamestate = firstMessage->getInstruction(i)->getGamestate();

            if (gamestate) {
                std::stringstream stream(gamestate->getConfigstring(21));
                if (!(stream >> ret))
                    throw DemoException("gamestate time extraction failed");
                return ret;
            }
        }
    }

    return -1;
}

int getSnapshotTime(Message* message) {
    if (!message)
        return -1;

    Snapshot* snapshot = getFirstSnapshot(message);

    if (snapshot)
        return snapshot->getServertime();

    return -1;
}

void Cutter::cutFromTime(Demo* demo, int time, int mapIndex) {
    assert(demo);
    assert(time >= 0);
    assert(mapIndex >= 0);

    int gameStateIndex;
    bool isRestart = demo->isMapRestart(mapIndex);

    if (time == 0 && !isRestart) {
        gameStateIndex = demo->getMapId(mapIndex);
        if (gameStateIndex > 0)
            demo->deleteMessage(0, gameStateIndex - 1);
        return;
    }

    int newMapindex = mapIndex;
    if (isRestart) {
        //find last precending non restart map
        while (demo->isMapRestart(newMapindex))
            --newMapindex;
    }
    gameStateIndex = demo->getMapId(newMapindex);

    demo->loadMessage(gameStateIndex);
    demo->loadMessage(gameStateIndex + 1);
    Gamestate* gamestate = 0;

    Message* tmpMessage = demo->getMessage(gameStateIndex);
    int id;
    for (id = 0; id < tmpMessage->getInstructionsCount(); ++id) {
        if (tmpMessage->getInstruction(id)->getType() == INSTR_GAMESTATE) {
            gamestate = tmpMessage->getInstruction(id)->getGamestate();
            break;
        }
    }

    if (id < tmpMessage->getInstructionsCount() - 1) {
        //there are some instructions after gamestate, delete
        tmpMessage->deleteInstruction(id + 1, tmpMessage->getInstructionsCount() - id - 1);
    }

    if (id > 0) {
        //there are some instructions before gamestate, delete
        tmpMessage->deleteInstruction(0, id);
    }

    if (!gamestate)
        throw DemoException("gamestate instruction not found in message");

    int starttime = getStartTime(demo, mapIndex);
    int i = gameStateIndex + 1;

    int nextMapMessageindex;

    if (mapIndex < demo->getMapsCount() - 1)
        nextMapMessageindex = demo->getMapId(mapIndex + 1);
    else
        nextMapMessageindex = demo->getMessageCount();

    int msgsCount = demo->getMessageCount();

    //int firstSnapshotIndex = demo->getMapId(mapIndex);
    //update our gamestate whole way to the selected time
    while (i < nextMapMessageindex) {
        demo->loadMessage(i);

        if (((time != 0 && (getSnapshotTime(demo->getMessage(i)) - starttime) >= time))
            || (time == 0 && isRestart && i == demo->getMapId(mapIndex))
            ) {
            break;
        }

        for (int j = 0; j < demo->getMessage(i)->getInstructionsCount(); ++j) {
            if (demo->getMessage(i)->getInstruction(j)->getType() == INSTR_SERVERCOMMAND)
                gamestate->update(demo->getMessage(i)->getInstruction(j)->getServerCommand());
        }

        if (i > gameStateIndex) //unload unless it is the initial gamestate message
            demo->unloadMessage(i);

        ++i;
    }
    /* i is now index to the very first snapshot for new demo */


    if (i == nextMapMessageindex)
        throw DemoException("starting time was not found");

    if (isRestart) {
        //for restart we need to update gamestate from
        //next message AND delete those server commands
        for (int j = 0; j < demo->getMessage(i)->getInstructionsCount(); ++j) {
            if (demo->getMessage(i)->getInstruction(j)->getType() == INSTR_SERVERCOMMAND)
                gamestate->update(demo->getMessage(i)->getInstruction(j)->getServerCommand());
        }

        //TO DO: delete unnecesarry server commands from next message 
        //(which should be first snapshot in new demo)
    }

    //we are gonna make the next message to be our first, not compressed
    int delta = getFirstSnapshot(demo->getMessage(i))->getDeltanum();
    int u = i - delta;
    int max_u, min_u;
    int seekingSeqNumber = demo->getMessage(i)->getSeqNumber() - delta;
    while (delta != 0) {

        demo->loadMessage(u);

        min_u = 0;
        max_u = msgsCount - 1;
        while (demo->getMessage(u) && min_u + 1 < max_u /* not infinite loop*/
            && demo->getMessage(u)->getSeqNumber() != seekingSeqNumber) {
            demo->unloadMessage(u);

            if (seekingSeqNumber < demo->getMessage(u)->getSeqNumber()) {
                //going down with u
                max_u = u;
                --u;
            }
            else {
                //going up with u
                min_u = u;
                ++u;
            }

            demo->loadMessage(u);
        }

        if (!demo->getMessage(u) || min_u + 1 == max_u) {
            std::string msg = "delta time resolving failed ";
            msg += "(";
            msg += seekingSeqNumber;
            msg += ")";
            throw DemoException(msg.c_str());
        }

        getFirstSnapshot(demo->getMessage(i))->applyOn(getFirstSnapshot(demo->getMessage(u)));

        delta = getFirstSnapshot(demo->getMessage(u))->getDeltanum();
        seekingSeqNumber = demo->getMessage(u)->getSeqNumber() - delta;
        demo->unloadMessage(u);
        u -= delta;

    }
    getFirstSnapshot(demo->getMessage(i))->setDeltanum(0);
    getFirstSnapshot(demo->getMessage(i))->makeInit();//removing null characters

    //need correction because of deltanums etc
    //not sure if this has to be here
    demo->loadMessage(i + 1);
    demo->getMessage(gameStateIndex)->setSeqNumber(demo->getMessage(i + 1)->getSeqNumber() - 2);
    demo->getMessage(i)->setSeqNumber(demo->getMessage(i + 1)->getSeqNumber() - 1);
    demo->getMessage(gameStateIndex)->setRelAcknowledge(demo->getMessage(i + 1)->getRelAcknowledge());
    demo->getMessage(i)->setRelAcknowledge(demo->getMessage(i + 1)->getRelAcknowledge());

    Snapshot* snap;
    for (int j = i + 1; j < i + 33; ++j) {//need to check only 32 snapshots at most
        if (j == nextMapMessageindex)
            break; // needed no more

        demo->loadMessage(j);

        snap = getFirstSnapshot(demo->getMessage(j));

        if (snap && (snap->getDeltanum() > (j - i))) {

            delta = getFirstSnapshot(demo->getMessage(j))->getDeltanum();
            seekingSeqNumber = demo->getMessage(j)->getSeqNumber() - delta;
            u = j - delta;
            while (delta != 0) {
                if (u <= 0)
                    throw DemoException("trying do delta from too old message");

                demo->loadMessage(u);

                min_u = 0;
                max_u = msgsCount - 1;
                while (demo->getMessage(u) && min_u + 1 < max_u /* not infinite loop*/
                    && demo->getMessage(u)->getSeqNumber() != seekingSeqNumber) {
                    demo->unloadMessage(u);

                    if (seekingSeqNumber < demo->getMessage(u)->getSeqNumber()) {
                        //going down with u
                        max_u = u;
                        --u;
                    }
                    else {
                        //going up with u
                        min_u = u;
                        ++u;
                    }

                    demo->loadMessage(u);
                }

                if (!demo->getMessage(u) || min_u + 1 == max_u) {
                    std::string msg = "delta time resolving failed ";
                    msg += "(";
                    msg += seekingSeqNumber;
                    msg += ")";
                    throw DemoException(msg.c_str());
                }

                snap->applyOn(getFirstSnapshot(demo->getMessage(u)));

                delta = getFirstSnapshot(demo->getMessage(u))->getDeltanum();

                //u or i
                seekingSeqNumber = demo->getMessage(u)->getSeqNumber() - delta;

                demo->unloadMessage(u);
                u -= delta;

            }
            snap->delta(getFirstSnapshot(demo->getMessage(i)));

            snap->setDeltanum(j - i);
        }
        else {
            demo->unloadMessage(j);
        }

    }

    demo->deleteMessage(gameStateIndex + 1, i);

    if (gameStateIndex > 0) {
        //not first map in demo, we also need to delete
        //all messages from beginning
        demo->deleteMessage(0, gameStateIndex);
    }
}

void Cutter::cutToTime(Demo* demo, int time, int mapIndex) {
    assert(demo);
    assert(time >= 0);
    assert(mapIndex >= 0);

    int starttime = getStartTime(demo, mapIndex);

    int snapTime;
    int i;

    if (time == 0) {
        if (mapIndex + 1 == demo->getMapsCount())
            return; //last map index, time 0, nothing to do

        //lets see when next map appears
        i = demo->getMapId(mapIndex + 1);

        //delete everything beginning with that map
        demo->deleteMessage(i, demo->getMessageCount());

        if (!demo->isMapRestart(mapIndex)) {
            //check last few messages and remove mapChange sequence
            int index = demo->getMessageCount() - 33;
            if (index < 0)
                index = 0;

            Message* msg;
            bool found = false;
            while (index < demo->getMessageCount()) {
                msg = demo->getMessage(index);
                for (int j = 0; j < msg->getInstructionsCount(); ++j) {
                    if (msg->getInstruction(j)->getType() == INSTR_MAPCHANGE) {
                        found = true;
                        break;
                    }
                }

                if (found)
                    break;

                ++index;
            }

            if (found)
                demo->deleteMessage(index, demo->getMessageCount());
        }

        return;
    }

    int nextMapMessageindex;

    if (mapIndex < demo->getMapsCount() - 1)
        nextMapMessageindex = demo->getMapId(mapIndex + 1);
    else
        nextMapMessageindex = demo->getMessageCount();

    i = demo->getMapId(mapIndex);
    while (i < nextMapMessageindex) {
        demo->loadMessage(i);

        snapTime = getSnapshotTime(demo->getMessage(i));
        if ((snapTime != -1) && ((snapTime - starttime) >= time))
            break;

        //TO DO: check some Changed variable instead
        if (i > 33) //these messages could be delta changed
            demo->unloadMessage(i);

        ++i;
    }

    if (i < nextMapMessageindex)
        demo->unloadMessage(i);

    demo->deleteMessage(i, demo->getMessageCount());

}