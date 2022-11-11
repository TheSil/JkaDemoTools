#include "demo.h"
#include "defs.h"

DEMO_NAMESPACE_START

class DemoImpl {
public:
    struct DemoRef { //12 bytes
        int      offset;
        Message* message;
        int      vehicleStatus;

        DemoRef() : message(0), vehicleStatus(VEHICLE_NOT_CHECKED) {
        };

    };

    std::string            demoName;
    std::ifstream          demoFile;
    std::vector<DemoRef>   messages;
    bool                   loaded;
    bool                   analysed;

    struct MapRef {
        int         messageId;
        std::string mapName;
        bool        isMapRestart;
        int         startTime;
        int         endTime;

        MapRef(int messageId, const std::string& mapName, bool isMapRestart)
            : messageId(messageId), mapName(mapName), isMapRestart(isMapRestart),
            startTime(0), endTime(0)
        {};
    };

    std::vector<MapRef> maps;

    Snapshot* getFirstSnapshot(Message* message);
    int getStartTime(Demo* demo, int mapIndex);
    int getSnapshotTime(Message* message);

    bool isValidIndex(int id);

};

bool DemoImpl::isValidIndex(int id) {
    return ((id >= 0) && (id < (int)messages.size()));
}

void Demo::saveMessage(int id, std::ofstream& os) const {
    if (!impl->isValidIndex(id))
        return;

    if (impl->messages[id].message && impl->messages[id].message->isLoad()) { //if message is loaded, write it from memory
        impl->messages[id].message->save(os);
    }
    else { //otherwise copy from source file
        impl->demoFile.seekg(impl->messages[id].offset, impl->demoFile.beg);

        int snumber, msglen;
        char buffer[MAX_MSGLEN];
        impl->demoFile.read((char*)&snumber, sizeof(snumber));
        impl->demoFile.read((char*)&msglen, sizeof(msglen));
        impl->demoFile.read((char*)&buffer, msglen);

        os.write((char*)&snumber, sizeof(snumber));
        os.write((char*)&msglen, sizeof(msglen));
        os.write((char*)&buffer, msglen);
    }
}

Demo::Demo() : impl(new DemoImpl())
{
    impl->loaded = false;
    impl->analysed = false;
}

Demo::~Demo() {
    close();
    delete impl;
}

bool Demo::open(const char* filename, bool analysis) {
    if (isOpen())
        close();

    impl->demoFile.open(filename, std::ios::binary);

    if (!impl->demoFile.is_open())
        return (impl->loaded = false);

    //log end offset
    impl->demoFile.seekg(0, std::ios_base::end);
    int end = (int)impl->demoFile.tellg();

    impl->demoFile.seekg(0, impl->demoFile.beg);

    int len;
    DemoImpl::DemoRef ref; //dummy ref

    do {
        ref.offset = (int)impl->demoFile.tellg();
        ref.message = 0;

        if (impl->demoFile.eof())
            continue;

        if (!impl->demoFile.fail())
            impl->demoFile.seekg(4, impl->demoFile.cur);
        if (!impl->demoFile.fail())
            impl->demoFile.read((char*)&len, 4);

        if ((int)impl->demoFile.tellg() > end - len)
            break; //truncated demo

        if (!impl->demoFile.fail() && (len != -1)) {
            impl->demoFile.seekg(len, impl->demoFile.cur);
            impl->messages.push_back(ref);
        }

    } while (!impl->demoFile.eof() && !impl->demoFile.fail() && (len != -1));

    impl->demoFile.clear();
    impl->analysed = false;

    return (impl->loaded = true);
}

void Demo::analyse() {

    if (impl->analysed)
        return;

    impl->maps.clear();
    Message::forceVehicleLoad = false;

    int lastSnapFlags = -1;
    int lastSnapTime = -1;
    bool awaitingMapChange = false;
    int count = getMessageCount();
    int messageId = 0;
    Message* msg;

    for (; messageId < count; ++messageId) {
        msg = getMessage(messageId);

        impl->messages[messageId].vehicleStatus = VEHICLE_NOT_CHECKED;

        if (!msg)
            continue;

        Message::forceVehicleLoad = false; //global

        Instruction* instr;
        for (int i = 0; i < msg->getInstructionsCount(); ++i) {
            instr = msg->getInstruction(i);

            assert(instr);

            if (instr->getType() == INSTR_SNAPSHOT) {
                //check snapshots for vehicle boarding/leaving or map restart

                /*
                Note: At this point, we might have loaded
                wrong packets entities instead of vehicle state,
                but that is not a problem, since we need to
                check only playerstate or pilotstate and they
                are loaded fine


                HOWEVER, we MUST check if there is not another instruction
                following after this one, because it might be incorrectly loaded
                */

                Snapshot* snap = instr->getSnapshot();

                assert(snap);

                lastSnapTime = snap->getServertime();

                //map restart check
                if (!awaitingMapChange) {
                    if ((lastSnapFlags != -1) &&
                        ((snap->getSnapflags() & 4) != lastSnapFlags)) {
                        //snap flag 4 switched => restart

                        //log ending time for previous map
                        int mapTime = impl->getStartTime(this, (int)impl->maps.size() - 1);
                        impl->maps[impl->maps.size() - 1].endTime = (lastSnapTime - mapTime) / 1000;

                        //insert
                        impl->maps.push_back(DemoImpl::MapRef(messageId, "restart", true));
                    }
                }
                lastSnapFlags = snap->getSnapflags() & 4;

                //vehicle check
                PlayerState* ps = snap->getPlayerstate();

                assert(ps);

                int vehicleId = (ps->getType() == STATE_PILOTSTATE) ? 31 : 84;

                //check for change in this snapshot
                if (ps->isAtributeSet(vehicleId)) {
                    if (ps->getAtributeInt(vehicleId))
                        impl->messages[messageId].vehicleStatus = VEHICLE_INSIDE;
                    else
                        impl->messages[messageId].vehicleStatus = VEHICLE_NOT_INSIDE;

                    continue;
                }

                //we still have a chance, if delta number is 0,
                //we have uncompressed frame and therefore we are not in vehicle
                //(otherwise previous check would detect it)
                int deltanum;
                deltanum = snap->getDeltanum();

                if (!deltanum) {
                    impl->messages[messageId].vehicleStatus = VEHICLE_NOT_INSIDE;
                    continue;
                }

                //actual value doesnt tell much, lets check delta value
                int guessingId, seekingSeqNumber, foundSeqNumber, maxId, minId;
                seekingSeqNumber = msg->getSeqNumber() - deltanum;
                guessingId = messageId - deltanum;

                //we keep minId <= guessingId < maxId, so we dont get 
                //into a loop
                maxId = messageId;
                minId = 0;

                do {
                    foundSeqNumber = getMessage(guessingId)->getSeqNumber();
                    //unloadMessage(guessingId);

                    if (seekingSeqNumber < foundSeqNumber) {
                        maxId = guessingId;
                        --guessingId;
                    }
                    else if (seekingSeqNumber > foundSeqNumber) {
                        ++guessingId;
                        minId = guessingId;
                    }

                } while ((seekingSeqNumber != foundSeqNumber) && (minId != maxId));


                if (minId == maxId) { //something went wrong, we didnt find seeking seq number
                    guessingId = messageId - deltanum;
                    impl->messages[guessingId].vehicleStatus = VEHICLE_NOT_INSIDE;
                }

                //we found matching message, get its vehicleStatus
                impl->messages[messageId].vehicleStatus = impl->messages[guessingId].vehicleStatus;

                if (impl->messages[messageId].vehicleStatus == VEHICLE_NOT_CHECKED) {
                    std::string s = "Vehicle status wasnt checked correctly ";
                    s += seekingSeqNumber;
                    throw DemoException(s.c_str());
                }

                if ((impl->messages[messageId].vehicleStatus == VEHICLE_INSIDE) &&
                    (!snap->getVehiclestate()) &&
                    (i < msg->getInstructionsCount() - 1)) {
                    //we are in vehicle, we didnt read snapshot properly AND
                    //according to readed information there is another instruction after this one
                    //we need reload
                    Message::forceVehicleLoad = true;
                    break;
                }


            }
            else if (instr->getType() == INSTR_GAMESTATE) {
                //check gamestates for new map beginning
                Gamestate* gamestate = instr->getGamestate();

                assert(gamestate);

                //get map name from configstring
                std::string s = gamestate->getConfigstring(0);

                int startIndex = (int)s.find("\\mapname\\");
                startIndex += 9;

                if (startIndex >= s.size())
                    continue; //not found or wrong format

                int endIndex = (int)s.find_first_of('\\', startIndex);

                if (endIndex == s.size())
                    continue; //not found (wrong format)

                //log ending time for previous map
                int currentTime = lastSnapTime;
                int mapTime;
                if (impl->maps.size() > 0) {
                    mapTime = impl->getStartTime(this, (int)impl->maps.size() - 1);
                    impl->maps[impl->maps.size() - 1].endTime = (currentTime - mapTime) / 1000;
                }

                //insert new map
                impl->maps.push_back(DemoImpl::MapRef(messageId,
                    s.substr(startIndex, endIndex - startIndex),
                    false));

                //log beginning time for new map
                currentTime = impl->getFirstSnapshot(getMessage(messageId + 1))->getServertime();
                mapTime = impl->getStartTime(this, (int)impl->maps.size() - 1);
                impl->maps[impl->maps.size() - 1].startTime = (currentTime - mapTime) / 1000;

                awaitingMapChange = false;

            }
            else if (instr->getType() == INSTR_MAPCHANGE) {
                awaitingMapChange = true;
            }

        }

        if ((impl->messages[messageId].vehicleStatus == VEHICLE_NOT_CHECKED)
            && messageId > 0) {
            //this is probably pure gamestate message, lets use vehicle status from previous message
            impl->messages[messageId].vehicleStatus = impl->messages[messageId - 1].vehicleStatus;

            if (impl->messages[messageId].vehicleStatus == VEHICLE_NOT_CHECKED) {
                std::string s = "Vehicle status wasnt checked correctly for msg ";
                s += messageId;
                throw DemoException(s.c_str());
            }

        }

        if (Message::forceVehicleLoad) {
            unloadMessage(messageId);
            --messageId;
        }
        else if (messageId - 16 >= 0) {
            unloadMessage(messageId - 16);
        }
    }

    //unload last 16 messages
    for (messageId = count - 16; messageId < count; ++messageId)
        unloadMessage(messageId);

    //log end time for last map
    int mapTime = impl->getStartTime(this, (int)impl->maps.size() - 1);
    impl->maps[impl->maps.size() - 1].endTime = (lastSnapTime - mapTime) / 1000;

    impl->analysed = true;
}

bool Demo::isOpen() const {
    return impl->loaded;
}

void Demo::close() {
    if (!isOpen())
        return;

    impl->loaded = false;
    impl->demoFile.close();
    impl->demoName.clear();

    for (std::vector<DemoImpl::DemoRef>::iterator it = impl->messages.begin();
        it != impl->messages.end(); ++it)
        if (it->message) delete it->message;

    impl->messages.clear();

    impl->maps.clear();
}

bool Demo::save(const char* filename, bool endSign) const {
    std::ofstream vystup(filename, std::ios::binary);

    if (!vystup.is_open())
        return false;

    for (int i = 0; i < getMessageCount(); ++i)
        saveMessage(i, vystup);

    if (endSign) {
        int end = -1;
        vystup.write((char*)&end, 4);
        vystup.write((char*)&end, 4);
    }

    vystup.close();
    return true;
}

void Demo::loadMessage(int id) {
    if (isMessageLoaded(id))
        return;

    impl->demoFile.seekg(impl->messages[id].offset, impl->demoFile.beg);

    //our vehicle magic
    if (impl->analysed) {
        if (impl->messages[id].vehicleStatus == VEHICLE_INSIDE)
            Message::forceVehicleLoad = true;
        else
            Message::forceVehicleLoad = false;
    }

    if (!impl->messages[id].message) {
        impl->messages[id].message = new Message();
    }

    if (impl->analysed) { //we did analysis, we can believe clean fast way
        impl->messages[id].message->load(impl->demoFile);
    }
    else {
        //not analysed, we must try eventually both variants (without and with vehicles)
        try {
            impl->messages[id].message->load(impl->demoFile);
        }
        catch (std::exception& e) {
            if (Message::forceVehicleLoad) {
                throw e;
            }
            else { //try again with forcing vehicle load
                Message::forceVehicleLoad = true;
                impl->messages[id].message->clear();
                impl->demoFile.seekg(impl->messages[id].offset, impl->demoFile.beg);
                impl->messages[id].message->load(impl->demoFile);
                Message::forceVehicleLoad = false;
            }
        }

    }

    //failed
    if (!impl->messages[id].message->isLoad()) {
        delete impl->messages[id].message;
        impl->messages[id].message = 0;
    }
}

bool Demo::isMessageLoaded(int id) const {
    if (!isOpen())
        return false;

    if ((id < 0) || (id > (int)(impl->messages.size() - 1)))
        return false;

    if (impl->messages[id].message && impl->messages[id].message->isLoad())
        return true;

    return false;
}

void Demo::unloadMessage(int id) {
    if (!isMessageLoaded(id))
        return;

    delete impl->messages[id].message;
    impl->messages[id].message = 0;
}

Message* Demo::getMessage(int id) {
    if (!isOpen())
        return 0;

    if ((id < 0) || (id > (int)(impl->messages.size() - 1)))
        return 0;

    loadMessage(id);

    if (!isMessageLoaded(id))
        return 0;


    return impl->messages[id].message;
}

int Demo::getMessageCount() const {
    return (int)impl->messages.size();
}

void Demo::deleteMessage(int startid, int endid) {
    if (!isOpen())
        return;

    if ((startid < 0) || (startid > (int)(impl->messages.size() - 1)))
        return;

    if (endid > startid) {
        if (endid >= (int)(impl->messages.size()))
            impl->messages.erase(impl->messages.begin() + startid, impl->messages.end());
        else
            impl->messages.erase(impl->messages.begin() + startid, impl->messages.begin() + endid);
    }
    else {
        if (impl->messages[startid].message) {
            delete impl->messages[startid].message;
        }

        impl->messages.erase(impl->messages.begin() + startid);
    }

}

//times extraction routines
Snapshot* DemoImpl::getFirstSnapshot(Message* message) {

    for (int i = 0; i < message->getInstructionsCount(); ++i)
        if (message->getInstruction(i)->getType() == INSTR_SNAPSHOT)
            return message->getInstruction(i)->getSnapshot();

    return 0;
}

int DemoImpl::getStartTime(Demo* demo, int mapIndex) {
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

int DemoImpl::getSnapshotTime(Message* message) {
    if (!message)
        return -1;

    Snapshot* snapshot = getFirstSnapshot(message);

    if (snapshot)
        return snapshot->getServertime();

    return -1;
}

//map offsets vector api
int Demo::getMapsCount() const {
    return (int)impl->maps.size();
}

std::string Demo::getMapName(int mapId) const {
    assert((mapId >= 0) && (mapId < impl->maps.size()));
    return impl->maps[mapId].mapName;
}

int Demo::getMapId(int mapId) const {
    assert((mapId >= 0) && (mapId < impl->maps.size()));
    return impl->maps[mapId].messageId;
}

bool Demo::isMapRestart(int mapId) const {
    assert((mapId >= 0) && (mapId < impl->maps.size()));
    return impl->maps[mapId].isMapRestart;
}

int Demo::getMapStartTime(int mapId) const {
    assert((mapId >= 0) && (mapId < impl->maps.size()));
    return impl->maps[mapId].startTime;
}

int Demo::getMapEndTime(int mapId) const {
    assert((mapId >= 0) && (mapId < impl->maps.size()));
    return impl->maps[mapId].endTime;
}

DEMO_NAMESPACE_END