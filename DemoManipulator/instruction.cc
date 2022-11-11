#include "instruction.h"

DEMO_NAMESPACE_START

/*

Instruction Implementation

*/

void Instruction::Save()  const {
}

void Instruction::Load() {
}

void Instruction::report(std::ostream& os) const {
    os << "*INSTRUCTION: Base Instruction (shouldnt happen)" << std::endl << std::endl;
}

/*

Instruction CONVERTING METHODS

*/

MapChange* Instruction::getMapChange() {
    if (type == INSTR_MAPCHANGE)
        return dynamic_cast<MapChange*>(this);

    return 0;
}

Gamestate* Instruction::getGamestate() {
    if (type == INSTR_GAMESTATE)
        return dynamic_cast<Gamestate*>(this);

    return 0;
}

Snapshot* Instruction::getSnapshot() {
    if (type == INSTR_SNAPSHOT)
        return dynamic_cast<Snapshot*>(this);

    return 0;
}

ServerCommand* Instruction::getServerCommand() {
    if (type == INSTR_SERVERCOMMAND)
        return dynamic_cast<ServerCommand*>(this);

    return 0;
}

/*

ServerCommand Implementation

*/

void ServerCommand::Save() const {
    Message::buffer.writeBits(svc_serverCommand, SIZE_8BITS);
    Message::buffer.writeBits(sequenceNumber, SIZE_32BITS);
    Message::buffer.writeString(command, false);
}

void ServerCommand::Load() {
    sequenceNumber = Message::buffer.readBits(SIZE_32BITS);
    command = Message::buffer.readString(true);
}

void ServerCommand::report(std::ostream& os) const {
    std::string s = command;

    for (std::string::iterator it = s.begin(); it != s.end(); ++it)
        if (*it == '\n')
            *it = ' ';

    os << "  *SERVER COMMAND*" << std::endl;
    os << "    Sequence number: " << sequenceNumber << std::endl;
    os << "    Command: \"" << s << "\"" << std::endl;
    os << std::endl;
}

Snapshot* Snapshot::clone() {
    Snapshot* snap = new Snapshot(*this);

    snap->playerState = playerState->clone();
    snap->vehicleState = 0; //make clone for vehicle states too!
    snap->entities = this->entities;

    return snap;
}

Snapshot::~Snapshot() {
    if (playerState)
        delete playerState;

    if (vehicleState)
        delete vehicleState;
}

void Snapshot::Save() const {
    Message::buffer.writeBits(svc_snapshot, SIZE_8BITS);
    Message::buffer.writeBits(serverTime, SIZE_32BITS);
    Message::buffer.writeBits(deltaNum, SIZE_8BITS);
    Message::buffer.writeBits(flags, SIZE_8BITS);

    Message::buffer.writeBits((int)areaMask.size(), SIZE_8BITS);
    for (std::vector<byte>::const_iterator it = areaMask.begin();
        it != areaMask.end(); ++it)
        Message::buffer.writeBits(*it, SIZE_8BITS);

    if (playerState->getType() == STATE_PLAYERSTATE) {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }
    else {
        Message::buffer.writeBits(1, SIZE_1BIT);
    }

    playerState->save();

    if (vehicleState)
        vehicleState->save();

    for (entitymap_cit it = entities.begin(); it != entities.end(); ++it) {
        Message::buffer.writeBits(it->first, SIZE_ENTITY_BITS);

        it->second.save();
    }
    Message::buffer.writeBits(1023, SIZE_ENTITY_BITS);
}

void Snapshot::Load() {
    serverTime = Message::buffer.readBits(SIZE_32BITS);
    deltaNum = Message::buffer.readBits(SIZE_8BITS);
    flags = Message::buffer.readBits(SIZE_8BITS);

    int len = Message::buffer.readBits(SIZE_8BITS);
    areaMask.resize(len);
    for (std::vector<byte>::iterator it = areaMask.begin();
        it != areaMask.end(); ++it)
        *it = Message::buffer.readBits(SIZE_8BITS);

    if (!Message::buffer.readBits(SIZE_1BIT))
        playerState = new PlayerState();
    else
        playerState = new PilotState();

    playerState->load();

    if (Message::forceVehicleLoad || playerState->hasVehicleSet()) {//load vehicle
        vehicleState = new VehicleState();
        vehicleState->load();
    }

    int testnumber;
    for (int i = 0; i < 1024; ++i) {
        testnumber = Message::buffer.readBits(SIZE_ENTITY_BITS);

        if (testnumber == 1023)
            break;

        if (testnumber < 0 || testnumber >= MAX_GENTITIES)
            throw DemoException("entity number out of range");
        //throw "entity number out of range";
        entities[testnumber].load();
    }
}

void Snapshot::report(std::ostream& os) const {
    os << "  *SNAPSHOT*" << std::endl;
    os << "    Server time: " << serverTime << std::endl;
    os << "    Delta number: " << deltaNum << std::endl;
    os << "    Snapflags: " << flags << std::endl;

    if (playerState->getType() == STATE_PILOTSTATE)
        os << std::endl << "    PILOT STATE CHANGES" << std::endl;
    else
        os << std::endl << "    PLAYER STATE CHANGES" << std::endl;

    playerState->report(os);

    if (vehicleState) {
        os << std::endl << "    VEHICLE STATE CHANGES" << std::endl;
        vehicleState->report(os);
    }

    if (!entities.empty()) {

        os << std::endl << "    PACKET ENTITIES";
        for (entitymap_cit it = entities.begin(); it != entities.end(); ++it) {
            os << std::endl << "    ENTITY " << it->first << " ";
            if (it->second.isRemoved()) os << " TO REMOVE";
            else if (it->second.noChanged()) os << " TO ADD";
            else it->second.report(os);
        }
    }
}

void Snapshot::delta(Snapshot* snap) {
    assert(snap);

    if (!playerState)
        playerState = new PlayerState();

    //HUGE BUG IN HERE
    //if we are creating delta of uncompressed snapshot, then 
    //any attribute that ISNT in THIS snapshot and IS in previous snapshot
    //SHOULD BE SET TO 0 !!!!

    playerState->delta(snap->playerState, snap->getDeltanum() == 0);

    if (snap->vehicleState && vehicleState) {
        vehicleState->delta(snap->vehicleState, snap->getDeltanum() == 0);
    }

    //update entities and get rid of that which we should remove
    for (entitymap_cit it = snap->entities.begin(); it != snap->entities.end(); ++it) {

        //entity was previously removed, if so, do nothing
        if (!it->second.isRemoved()) {
            //we have this entity for change
            if (entities.find(it->first) != entities.end()) {
                if (!entities[it->first].isRemoved())
                    entities[it->first].delta(&it->second);
            }
            else if (getDeltanum() == 0) {
                //we havent found this stats 
                entities[it->first].setRemove(true);
            }
        }
    }

}

void Snapshot::applyOn(Snapshot* snap) {
    assert(snap);

    if (!playerState)
        playerState = new PlayerState();

    playerState->applyOn(snap->playerState);

    if (snap->vehicleState && vehicleState) {
        vehicleState->applyOn(snap->vehicleState);
    }

    //TO DO: apply support for vehicleState

    //update entities and get rid of that which we should remove
    for (entitymap_cit it = snap->entities.begin(); it != snap->entities.end(); ++it) {

        //entity was previously removed, if so, do nothing
        if (!it->second.isRemoved()) {
            //we are not changing this entity here, so load old
            if (entities.find(it->first) == entities.end()) {
                entities[it->first] = it->second;
            }
            else {
                if (!entities[it->first].isRemoved()
                    && !entities[it->first].getprevtoremove())
                    entities[it->first].applyOn(&it->second);
            }
        }
        else {
            if (entities.find(it->first) == entities.end()) {
                entities[it->first].setRemove(true);
            }
            else {
                entities[it->first].setprevtoremove(true);
            }

        }

    }

}

void Snapshot::makeInit() {
    playerState->removeNull();

    //get rid of entities ordered to remove
    for (entitymap_it it = entities.begin(); it != entities.end();) {
        if (it->second.isRemoved()) {

            entities.erase(it++);
        }
        else {
            it->second.removeNull(); //get rid of null informations
            ++it;
        }
    }

}

void Snapshot::removeNotChanged() {

    for (entitymap_it it = entities.begin(); it != entities.end();) {
        if (it->second.noChanged() && !it->second.isRemoved()) {
            entities.erase(it++);

        }
        else {
            ++it;
        }
    }

}

/*

Gamestate Implementation

*/

void Gamestate::Save() const {
    Message::buffer.writeBits(svc_gamestate, SIZE_8BITS);
    Message::buffer.writeBits(commandSequence, SIZE_32BITS);

    //writing configstrings
    for (stringmap_cit it = configStrings.begin(); it != configStrings.end(); ++it) {
        if (!it->second.empty()) {
            Message::buffer.writeBits(svc_configstring, SIZE_8BITS);
            Message::buffer.writeBits(it->first, SIZE_16BITS);
            Message::buffer.writeString(it->second, true);
        }
    }

    //writing baseline entities
    for (entitymap_cit it = baseEntities.begin(); it != baseEntities.end(); ++it) {
        Message::buffer.writeBits(svc_baseline, SIZE_8BITS);
        Message::buffer.writeBits(it->first, SIZE_ENTITY_BITS);
        it->second.save();
    }

    //end of gamestate message
    Message::buffer.writeBits(svc_EOF, SIZE_8BITS);
    Message::buffer.writeBits(clientNumber, SIZE_32BITS);
    Message::buffer.writeBits(checksumFeed, SIZE_32BITS);

    Message::buffer.writeBits((int)magicStuff.size(), SIZE_16BITS);

    if (magicStuff.size() > 0) {
        Message::buffer.writeBits(0, SIZE_1BIT); //wtf

        for (std::string::const_iterator it = magicStuff.begin(); it != magicStuff.end(); ++it) {
            Message::buffer.writeBits(*it, SIZE_8BITS);
        }

        Message::buffer.writeBits((int)magicStuff.size(), SIZE_16BITS); //wtf
        Message::buffer.writeBits(0, SIZE_1BIT); //wtf

        if (magicStuff.size() > 0) { //wtf
            for (std::string::const_iterator it = magicStuff.begin(); it != magicStuff.end(); ++it) {
                Message::buffer.writeBits(*it, SIZE_8BITS);
            }
        }

        Message::buffer.writeBits(magicSeed, SIZE_32BITS);
        Message::buffer.writeBits((int)magicData.size(), SIZE_16BITS);

        if (magicData.size() > 0) {
            for (std::vector<MagicData>::const_iterator it = magicData.begin(); it != magicData.end(); ++it) {
                Message::buffer.writeBits(it->byte1, SIZE_8BITS);
                Message::buffer.writeBits(it->byte2, SIZE_8BITS);
                Message::buffer.writeBits(it->int1, SIZE_32BITS);
                Message::buffer.writeBits(it->int2, SIZE_32BITS);
            }
        }

    }
}

void Gamestate::Load() {
    //server command sequence
    commandSequence = Message::buffer.readBits(SIZE_32BITS);

    int cmd;
    while (true) {
        cmd = Message::buffer.readBits(SIZE_8BITS);

        if (cmd == svc_EOF)
            break;

        if (cmd == svc_configstring) {
            int i = Message::buffer.readBits(SIZE_16BITS);

            if (i < 0 || i >= MAX_CONFIGSTRINGS)
                throw DemoException("configstring id out of range");

            configStrings[i] = Message::buffer.readString(true);
        }
        else if (cmd == svc_baseline) {
            int newnum = Message::buffer.readBits(SIZE_ENTITY_BITS);

            if (newnum < 0 || newnum >= MAX_GENTITIES)
                throw DemoException("entity number out of range");

            baseEntities[newnum].load();
        }
        else {
            throw DemoException("unknown message type (inside gamestate)");
        }

    }

    clientNumber = Message::buffer.readBits(SIZE_32BITS);
    checksumFeed = Message::buffer.readBits(SIZE_32BITS);

    magicStuff.reserve(Message::buffer.readBits(SIZE_16BITS));

    if (magicStuff.size() > 0) {
        Message::buffer.readBits(SIZE_1BIT); //wtf

        for (std::string::iterator it = magicStuff.begin(); it != magicStuff.end(); ++it) {
            *it = Message::buffer.readBits(SIZE_8BITS);
        }

        Message::buffer.readBits(SIZE_16BITS); //wtf
        Message::buffer.readBits(SIZE_1BIT); //wtf

        if (magicStuff.size() > 0) { //wtf
            for (std::string::iterator it = magicStuff.begin(); it != magicStuff.end(); ++it) {
                *it = Message::buffer.readBits(SIZE_8BITS);
            }
        }

        magicSeed = Message::buffer.readBits(SIZE_32BITS);
        magicData.resize(Message::buffer.readBits(SIZE_16BITS));

        if (magicData.size() > 0) {
            for (std::vector<MagicData>::iterator it = magicData.begin(); it != magicData.end(); ++it) {
                it->byte1 = Message::buffer.readBits(SIZE_8BITS);
                it->byte2 = Message::buffer.readBits(SIZE_8BITS);
                it->int1 = Message::buffer.readBits(SIZE_32BITS);
                it->int2 = Message::buffer.readBits(SIZE_32BITS);
            }
        }

    }
}

void Gamestate::report(std::ostream& os) const {
    os << "  *GAMESTATE*" << std::endl;
    os << "    Command sequence number: " << commandSequence;
    os << " Client number: " << clientNumber;
    os << " Checksumfeed: " << checksumFeed;
    os << " Magic stuff length: " << magicStuff.size();
    os << std::endl;

    os << "    GAMESTATE CONFIGSTRINGS" << std::endl;
    for (stringmap_cit it = configStrings.begin(); it != configStrings.end(); ++it)
        os << "    configstring(" << it->first << "): " << it->second << std::endl;
    os << std::endl;


    os << "    GAMESTATE BASELINE ENTITIES" << std::endl;
    for (entitymap_cit it = baseEntities.begin(); it != baseEntities.end(); ++it) {
        os << "    ENTITY " << it->first << " ";
        if (it->second.isRemoved()) os << " TO REMOVE";
        else if (it->second.noChanged()) os << " NOT CHANGED";
        else it->second.report(os);
        os << std::endl;
    }
}

std::string Gamestate::getConfigstring(int id) {
    if (id < 0 || id >= MAX_CONFIGSTRINGS)
        throw DemoException("configstring id out of range");

    return configStrings[id];
}

void Gamestate::removeConfigstring(int id) {
    stringmap_it it = configStrings.find(id);

    if (it != configStrings.end()) {
        configStrings.erase(it);
    }
}

std::string Gamestate::getMagicStuff() {
    return magicStuff;
}

int Gamestate::getMagicSeed() {
    return magicSeed;
}

int Gamestate::getMagicDataCount() {
    return (int)magicData.size();
}

void Gamestate::getMagicData(unsigned id, int* byte1, int* byte2, int* int1, int* int2) {
    if (id >= magicData.size())
        throw DemoException("magic data index out of range");

    if (byte1) *byte1 = magicData[id].byte1;
    if (byte2) *byte2 = magicData[id].byte2;
    if (int1) *int1 = magicData[id].int1;
    if (int2) *int2 = magicData[id].int2;
}

void Gamestate::setConfigstring(int id, const std::string& s) {
    if (id < 0 || id >= MAX_CONFIGSTRINGS)
        throw DemoException("configstring id out of range");

    configStrings[id] = s;
}

void Gamestate::setMagicStuff(const std::string& s) {
    magicStuff = s;
}

void Gamestate::setMagicSeed(int seed) {
    magicSeed = seed;
}

void Gamestate::setMagicData(unsigned id, int byte1, int byte2, int int1, int int2) {
    if (magicData.size() <= id) {
        magicData.resize(id + 1);
    }

    magicData[id].byte1 = byte1;
    magicData[id].byte2 = byte2;
    magicData[id].int1 = int1;
    magicData[id].int2 = int2;
}

void Gamestate::update(const ServerCommand* servercommand) {
    assert(servercommand);

    const std::string command = servercommand->getCommand();
    if (command.find("cs ") == 0) {
        int i;

        int start = (int)command.find_first_of(' ', 3);
        std::stringstream stream(command.substr(3, start - 3));
        stream >> i;

        start = (int)command.find_first_of('"', start);
        int end = (int)command.find_first_of('"', start + 1);

        setConfigstring(i, command.substr(start + 1, end - start - 1));
    }
}

/*

MapChange Implementation

*/

void MapChange::Save() const {
    Message::buffer.writeBits(svc_mapchange, SIZE_8BITS);
}

void MapChange::report(std::ostream& os) const {
    os << "  *MAP CHANGE*" << std::endl << std::endl;
}

DEMO_NAMESPACE_END