#include "state.h"

DEMO_NAMESPACE_START

float State::getAtributeFloat(int id) const {
    IntAtributeMapCit it = atributes.find(id);
    if (it != atributes.end())
        return it->second.fVal;

    return 0;
}

int State::getAtributeInt(int id) const {
    IntAtributeMapCit it = atributes.find(id);
    if (it != atributes.end())
        return it->second.iVal;

    return 0;
}

void State::setAtribute(int id, float value) {
    atributes[id].fVal = value;
}

void State::setAtribute(int id, int value) {
    atributes[id].iVal = value;
}

bool State::isAtributeSet(int id) const {
    return (atributes.find(id) != (atributes.end()));
}

int State::getAtributesCount() {
    return (int)atributes.size();
}

PlayerState* State::getPlayerstate() {
    if (getType() != STATE_PLAYERSTATE && getType() != STATE_PILOTSTATE)
        return 0;

    return dynamic_cast<PlayerState*>(this);
}

void State::clear() {
    atributes.clear();
}

State* EntityState::clone() {
    EntityState* returnEntityState = new EntityState();

    returnEntityState->atributes = this->atributes;
    returnEntityState->toRemove = this->toRemove;
    returnEntityState->previousToRemove = this->previousToRemove;

    return returnEntityState;
}

void EntityState::save() const {
    //to remove bit
    if (toRemove) {
        Message::buffer.writeBits(1, SIZE_1BIT);
        return;
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }

    //emptiness bit
    if (atributes.empty()) {
        Message::buffer.writeBits(0, SIZE_1BIT);
        return;
    }
    else {
        Message::buffer.writeBits(1, SIZE_1BIT);
    }

    //last changed byte
    Message::buffer.writeBits(((atributes.rbegin())->first) + 1, SIZE_8BITS);

    int nulled = 0;
    int bufiVal; float buffVal;

    for (std::map<int, Atribute>::const_iterator it = atributes.begin();
        it != atributes.end(); ++it) {
        //null previous atributes
        for (; nulled != it->first; ++nulled) Message::buffer.writeBits(0, SIZE_1BIT);

        //here comes change
        Message::buffer.writeBits(1, SIZE_1BIT);

        bufiVal = it->second.iVal;
        buffVal = it->second.fVal;

        if (EntityNetfield[it->first].type == FIELD_FLOAT) {
            //float number

            if (buffVal == 0.0f) {
                Message::buffer.writeBits(0, SIZE_1BIT);
            }
            else {
                Message::buffer.writeBits(1, SIZE_1BIT);

                int buffValtrunc = (int)buffVal;

                if ((buffValtrunc == buffVal) && (buffValtrunc + FLOAT_INT_BIAS >= 0)
                    && buffValtrunc + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS)) {
                    Message::buffer.writeBits(0, SIZE_1BIT);
                    Message::buffer.writeBits(buffValtrunc + FLOAT_INT_BIAS, SIZE_FLOATINT);
                }
                else {
                    Message::buffer.writeBits(1, SIZE_1BIT);
                    Message::buffer.writeBits(bufiVal, SIZE_32BITS);
                }
            }

        }
        else {
            //integer
            if (bufiVal == 0) { //0
                Message::buffer.writeBits(0, SIZE_1BIT);
            }
            else {
                Message::buffer.writeBits(1, SIZE_1BIT);
                Message::buffer.writeBits(bufiVal, EntityNetfield[it->first].type);
            }

        }

        ++nulled;
    }
}

void EntityState::load() {
    clear();

    // 1st bit tells us to remove entity
    if (Message::buffer.readBits(SIZE_1BIT)) {
        toRemove = true;
        return;
    }

    // 2nd bit tells if theres no change actually..
    if (Message::buffer.readBits(SIZE_1BIT) == 0) {
        return;
    }

    //next byte gives upper bound of changed stats
    int lastchanged = Message::buffer.readBits(SIZE_8BITS);

    int size = sizeof(EntityNetfield) / sizeof(Field);

    //now we read atributes one by one
    for (int i = 0; i < lastchanged; i++) {
        if (i < 0 || i >= size)
            throw DemoException("entitystate index out of range");

        if (Message::buffer.readBits(SIZE_1BIT)) { //something changed here
            if (EntityNetfield[i].type == FIELD_FLOAT) {
                //float number
                if (!Message::buffer.readBits(SIZE_1BIT)) {
                    atributes[i].fVal = 0.0f;
                }
                else {
                    if (!Message::buffer.readBits(SIZE_1BIT)) {
                        //integral float
                        atributes[i].fVal = (float)Message::buffer.readBits(FLOAT_INT_BITS);
                        atributes[i].fVal -= FLOAT_INT_BIAS;
                    }
                    else {
                        //full floating point
                        atributes[i].iVal = Message::buffer.readBits(SIZE_32BITS);
                    }
                }
            }
            else {
                //integer
                if (!Message::buffer.readBits(SIZE_1BIT)) {
                    atributes[i].iVal = 0;
                }
                else {
                    atributes[i].iVal = Message::buffer.readBits(EntityNetfield[i].type);
                }
            }
        }
    }
}

void EntityState::report(std::ostream& os) const {
    if (toRemove) {
        os << "ORDER TO REMOVE FROM CLIENT" << std::endl;
        return;
    }
    for (std::map<int, Atribute>::const_iterator it = atributes.begin();
        it != atributes.end(); ++it) {
        os << EntityNetfield[it->first]._name << ": ";

        if ((EntityNetfield[it->first].type == FIELD_FLOAT))
            os << it->second.fVal;
        else
            os << it->second.iVal;
        os << " ";
    }
}

bool EntityState::noChanged() const {
    return (atributes.empty());
}

bool EntityState::isChanged() const {
    return true;
}

bool EntityState::isAtributeFloat(int id) const {
    return (EntityNetfield[id].type == FIELD_FLOAT);
}

bool EntityState::isAtributeInteger(int id) const {
    return !isAtributeFloat(id);
}

void EntityState::delta(const EntityState* state) {
    for (std::map<int, Atribute>::const_iterator it = state->atributes.begin();
        it != state->atributes.end(); ++it) {

        if (atributes.find(it->first) != atributes.end()) {
            //atribute from previous entity exists in current entity
            if (atributes[it->first].iVal == it->second.iVal)
                atributes.erase(it->first);
        }
        else {
            //atribute from previous entity doest not exist in current entity
            setAtribute(it->first, 0);
        }
    }
}

void EntityState::applyOn(const EntityState* state) {
    for (std::map<int, Atribute>::const_iterator it = state->atributes.begin();
        it != state->atributes.end(); ++it) {

        if (atributes.find(it->first) == atributes.end())
            atributes[it->first] = it->second;
    }

}

void EntityState::removeNull() {
    IntAtributeMap tempMap;

    for (IntAtributeMapCit it = atributes.begin();
        it != atributes.end(); ++it) {
        if ((EntityNetfield[it->first].type != FIELD_FLOAT || it->second.fVal != 0.0f)
            && (EntityNetfield[it->first].type == FIELD_FLOAT || it->second.iVal != 0)) {
            tempMap[it->first] = it->second;
        }
    }
    atributes.swap(tempMap);
}

void EntityState::clear() {
    State::clear();

    toRemove = false;
    previousToRemove = false;
}

PlayerState* PlayerState::clone() {
    PlayerState* ps = new PlayerState();

    ps->atributes = this->atributes;
    ps->stats = this->stats;
    ps->persistant = this->persistant;
    ps->ammo = this->ammo;
    ps->powerups = this->powerups;

    return ps;
}

void PlayerState::save() const {
    //last changed byte
    if (!atributes.empty())
        Message::buffer.writeBits(((atributes.rbegin())->first) + 1, SIZE_8BITS);
    else
        Message::buffer.writeBits(0, SIZE_8BITS);

    int nulled = 0;
    int bufiVal; float buffVal;

    for (std::map<int, Atribute>::const_iterator it = atributes.begin();
        it != atributes.end(); ++it) {
        //null previous atributes
        for (; nulled != it->first; ++nulled) Message::buffer.writeBits(0, SIZE_1BIT);

        //here comes change
        Message::buffer.writeBits(1, SIZE_1BIT);

        bufiVal = it->second.iVal;
        buffVal = it->second.fVal;

        if (PlayerNetfield[it->first].type == FIELD_FLOAT) {
            //float number
            int buffValtrunc = (int)buffVal;

            if ((buffValtrunc == buffVal) && (buffValtrunc + FLOAT_INT_BIAS >= 0)
                && buffValtrunc + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS)) {
                Message::buffer.writeBits(0, SIZE_1BIT);
                Message::buffer.writeBits(buffValtrunc + FLOAT_INT_BIAS, SIZE_FLOATINT);
            }
            else {
                Message::buffer.writeBits(1, SIZE_1BIT);
                Message::buffer.writeBits(bufiVal, SIZE_32BITS);
            }

        }
        else {
            //integer
            Message::buffer.writeBits(bufiVal, PlayerNetfield[it->first].type);
        }

        ++nulled;
    }

    if (!stats.empty() || !persistant.empty()
        || !ammo.empty() || !powerups.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
        return;
    }

    int bits; statsarray_cit it;

    if (!stats.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = stats.begin(); it != stats.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = stats.begin(); it != stats.end(); ++it) {
            if (it->first == 4) Message::buffer.writeBits(it->second, SIZE_19BITS);
            else Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }

    if (!persistant.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = persistant.begin(); it != persistant.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = persistant.begin(); it != persistant.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }


    if (!ammo.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = ammo.begin(); it != ammo.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = ammo.begin(); it != ammo.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }

    if (!powerups.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = powerups.begin(); it != powerups.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = powerups.begin(); it != powerups.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_32BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }
}

void PlayerState::load() {
    clear();

    //first byte gives upper bound of changed stats
    int lastchanged = Message::buffer.readBits(SIZE_8BITS);

    int size = sizeof(PlayerNetfield) / sizeof(Field);

    //now we read atributes one by one
    for (int i = 0; i < lastchanged; i++) {
        if (i < 0 || i >= size)
            throw DemoException("playerstate index out of range");

        if (Message::buffer.readBits(1)) { //something changed here
            if (PlayerNetfield[i].type == FIELD_FLOAT) {
                //float number
                if (!Message::buffer.readBits(SIZE_1BIT)) {
                    //integral float
                    atributes[i].fVal = (float)Message::buffer.readBits(FLOAT_INT_BITS);
                    atributes[i].fVal -= FLOAT_INT_BIAS;
                }
                else {
                    //full floating point
                    atributes[i].iVal = Message::buffer.readBits(SIZE_32BITS);
                }
            }
            else {
                //integer
                atributes[i].iVal = Message::buffer.readBits(PlayerNetfield[i].type);
            }
        }
    }

    //nacitani statsu
    if (Message::buffer.readBits(SIZE_1BIT)) {
        int bits;

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) stats[i] = (i == 4) ? Message::buffer.readBits(SIZE_19BITS)
                    : Message::buffer.readBits(SIZE_16BITS);
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) persistant[i] = Message::buffer.readBits(SIZE_16BITS);
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) ammo[i] = Message::buffer.readBits(SIZE_16BITS);
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) powerups[i] = Message::buffer.readBits(SIZE_32BITS);
        }

    }
}

void PlayerState::report(std::ostream& os) const {
    os << "    ";
    for (std::map<int, Atribute>::const_iterator it = atributes.begin();
        it != atributes.end(); ++it) {
        os << PlayerNetfield[it->first]._name << ": ";

        if ((PlayerNetfield[it->first].type == FIELD_FLOAT))
            os << it->second.fVal;
        else
            os << it->second.iVal;
        os << " ";
    }
    os << std::endl;

    if (!stats.empty() || !persistant.empty()
        || !ammo.empty() || !powerups.empty()) {
        os << "    ";
        for (statsarray_cit it = stats.begin(); it != stats.end(); ++it)
            os << "ps_stats(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = persistant.begin();
            it != persistant.end(); ++it)
            os << "ps_persistant(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = ammo.begin(); it != ammo.end(); ++it)
            os << "ps_ammo(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = powerups.begin(); it != powerups.end(); ++it)
            os << "ps_powerups(" << it->first << "): " << it->second << " ";
        os << std::endl;
    }
}

bool PlayerState::noChanged() const {
    return (atributes.empty() && stats.empty() && persistant.empty()
        && ammo.empty() && powerups.empty());
}

bool PlayerState::hasVehicleSet() const {
    return isAtributeSet(84) && ((getAtributeInt(84) != 0)); //m_iVehicleNum
}

bool PlayerState::isChanged() const {
    return true;
}

bool PlayerState::isAtributeFloat(int id) const {
    return (PlayerNetfield[id].type == FIELD_FLOAT);
}

bool PlayerState::isAtributeInteger(int id) const {
    return !isAtributeFloat(id);
}

void PlayerState::delta(const PlayerState* state, bool isUncompressed)
{
    int test;

    //update player's informations
    for (std::map<int, Atribute>::const_iterator it = state->atributes.begin();
        it != state->atributes.end(); ++it) {

        test = it->first;

        //if this snaps doesnt tell us to change this atribute
        //we add old value from state
        if (atributes.find(it->first) != atributes.end()) {
            //we wanna set this value,lets check if its not already the same
            if (atributes[it->first].iVal == it->second.iVal)
                atributes.erase(it->first);
        }
        else if (isUncompressed) {
            //atribute isnt in this uncompressed snap, so we should create it with value 0
            atributes[it->first].iVal = 0;

        }
    }

    //update arrays
    statsarray_cit it;
    for (it = state->stats.begin();
        it != state->stats.end(); ++it) {

        if (stats.find(it->first) != stats.end()) {
            if (stats[it->first] == it->second)
                stats.erase(it->first);
        }
        else if (isUncompressed) {
            stats[it->first] = 0;
        }
    }

    for (it = state->persistant.begin();
        it != state->persistant.end(); ++it) {

        if (persistant.find(it->first) != persistant.end()) {
            if (persistant[it->first] == it->second)
                persistant.erase(it->first);
        }
        else if (isUncompressed) {
            persistant[it->first] = 0;
        }
    }

    for (it = state->ammo.begin();
        it != state->ammo.end(); ++it) {

        if (ammo.find(it->first) != ammo.end()) {
            if (ammo[it->first] == it->second)
                ammo.erase(it->first);
        }
        else if (isUncompressed) {
            ammo[it->first] = 0;
        }
    }

    for (it = state->powerups.begin();
        it != state->powerups.end(); ++it) {

        if (powerups.find(it->first) != powerups.end()) {
            if (powerups[it->first] == it->second)
                powerups.erase(it->first);
        }
        else if (isUncompressed) {
            powerups[it->first] = 0;
        }
    }
}

void PlayerState::applyOn(PlayerState* state) {
    //update player's informations
    for (std::map<int, Atribute>::const_iterator it = state->atributes.begin();
        it != state->atributes.end(); ++it) {

        //if this snaps doesnt tell us to change this atribute
        //we add old value from state
        if (atributes.find(it->first) == atributes.end())
            atributes[it->first] = it->second;
    }

    //update arrays
    statsarray_cit it;
    for (it = state->stats.begin();
        it != state->stats.end(); ++it) {

        if (stats.find(it->first) == stats.end())
            stats[it->first] = it->second;
    }

    for (it = state->persistant.begin();
        it != state->persistant.end(); ++it) {

        if (persistant.find(it->first) == persistant.end())
            persistant[it->first] = it->second;
    }

    for (it = state->ammo.begin();
        it != state->ammo.end(); ++it) {

        if (ammo.find(it->first) == ammo.end())
            ammo[it->first] = it->second;
    }

    for (it = state->powerups.begin();
        it != state->powerups.end(); ++it) {

        if (powerups.find(it->first) == powerups.end())
            powerups[it->first] = it->second;
    }
}

void PlayerState::removeNull() {
    IntAtributeMap tempMap;

    for (IntAtributeMapCit it = atributes.begin();
        it != atributes.end(); ++it) {
        if ((PlayerNetfield[it->first].type != FIELD_FLOAT || it->second.fVal != 0.0f)
            && (PlayerNetfield[it->first].type == FIELD_FLOAT || it->second.iVal != 0)) {
            tempMap[it->first] = it->second;
        }
    }
    atributes.swap(tempMap);

    statsarray tempStats;
    for (statsarray_cit it = stats.begin(); it != stats.end(); ++it) {
        if (it->second != 0)
            tempStats[it->first] = it->second;
    }
    stats.swap(tempStats);
    tempStats.clear();

    for (statsarray_cit it = persistant.begin(); it != persistant.end(); ++it) {
        if (it->second != 0)
            tempStats[it->first] = it->second;
    }
    persistant.swap(tempStats);
    tempStats.clear();

    for (statsarray_cit it = ammo.begin(); it != ammo.end(); ++it) {
        if (it->second != 0)
            tempStats[it->first] = it->second;
    }
    ammo.swap(tempStats);
    tempStats.clear();

    for (statsarray_cit it = powerups.begin(); it != powerups.end(); ++it) {
        if (it->second != 0)
            tempStats[it->first] = it->second;
    }
    powerups.swap(tempStats);

}

void PlayerState::clear() {
    State::clear();

    stats.clear();
    persistant.clear();
    ammo.clear();
    powerups.clear();
}

void PilotState::save() const {
    //last changed byte
    if (!atributes.empty())
        Message::buffer.writeBits(((atributes.rbegin())->first) + 1, SIZE_8BITS);
    else
        Message::buffer.writeBits(0, SIZE_8BITS);

    int nulled = 0;
    int bufiVal; float buffVal;

    for (std::map<int, Atribute>::const_iterator it = atributes.begin();
        it != atributes.end(); ++it) {
        //null previous atributes
        for (; nulled != it->first; ++nulled) Message::buffer.writeBits(0, SIZE_1BIT);

        //here comes change
        Message::buffer.writeBits(1, SIZE_1BIT);

        bufiVal = it->second.iVal;
        buffVal = it->second.fVal;

        if (PilotNetfield[it->first].type == FIELD_FLOAT) {
            //float number
            int buffValtrunc = (int)buffVal;

            if ((buffValtrunc == buffVal) && (buffValtrunc + FLOAT_INT_BIAS >= 0)
                && buffValtrunc + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS)) {
                Message::buffer.writeBits(0, SIZE_1BIT);
                Message::buffer.writeBits(buffValtrunc + FLOAT_INT_BIAS, SIZE_FLOATINT);
            }
            else {
                Message::buffer.writeBits(1, SIZE_1BIT);
                Message::buffer.writeBits(bufiVal, SIZE_32BITS);
            }

        }
        else {
            //integer
            Message::buffer.writeBits(bufiVal, PilotNetfield[it->first].type);
        }

        ++nulled;
    }

    if (!stats.empty() || !persistant.empty()
        || !ammo.empty() || !powerups.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
        return;
    }

    int bits; statsarray_cit it;

    if (!stats.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = stats.begin(); it != stats.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = stats.begin(); it != stats.end(); ++it) {
            if (it->first == 4) Message::buffer.writeBits(it->second, SIZE_19BITS);
            else Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }

    if (!persistant.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = persistant.begin(); it != persistant.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = persistant.begin(); it != persistant.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }

    if (!ammo.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = ammo.begin(); it != ammo.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = ammo.begin(); it != ammo.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }

    if (!powerups.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = powerups.begin(); it != powerups.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = powerups.begin(); it != powerups.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_32BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }
}

void PilotState::load() {
    clear();

    //first byte gives upper bound of changed stats
    int lastchanged = Message::buffer.readBits(SIZE_8BITS);

    int size = sizeof(PilotNetfield) / sizeof(Field);

    //now we read atributes one by one
    for (int i = 0; i < lastchanged; i++) {
        if (i < 0 || i >= size)
            throw DemoException("pilotstate index out of range");

        if (Message::buffer.readBits(1)) { //something changed here
            if (PilotNetfield[i].type == FIELD_FLOAT) {
                //float number
                if (!Message::buffer.readBits(SIZE_1BIT)) {
                    //integral float
                    atributes[i].fVal = (float)Message::buffer.readBits(FLOAT_INT_BITS);
                    atributes[i].fVal -= FLOAT_INT_BIAS;
                }
                else {
                    //full floating point
                    atributes[i].iVal = Message::buffer.readBits(SIZE_32BITS);
                }
            }
            else {
                //integer
                atributes[i].iVal = Message::buffer.readBits(PilotNetfield[i].type);
            }
        }
    }

    //stats loading
    if (Message::buffer.readBits(SIZE_1BIT)) {
        int bits;

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) stats[i] = (i == 4) ? Message::buffer.readBits(SIZE_19BITS)
                    : Message::buffer.readBits(SIZE_16BITS);
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) persistant[i] = Message::buffer.readBits(SIZE_16BITS);
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) ammo[i] = Message::buffer.readBits(SIZE_16BITS);
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) powerups[i] = Message::buffer.readBits(SIZE_32BITS);
        }
    }
}

void PilotState::report(std::ostream& os) const {
    os << "    ";
    for (std::map<int, Atribute>::const_iterator it = atributes.begin();
        it != atributes.end(); ++it) {
        os << PilotNetfield[it->first]._name << ": ";

        if ((PilotNetfield[it->first].type == FIELD_FLOAT))
            os << it->second.fVal;
        else
            os << it->second.iVal;
        os << " ";
    }
    os << std::endl;

    if (!stats.empty() || !persistant.empty()
        || !ammo.empty() || !powerups.empty()) {
        os << "    ";
        for (statsarray_cit it = stats.begin(); it != stats.end(); ++it)
            os << "ps_stats(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = persistant.begin();
            it != persistant.end(); ++it)
            os << "ps_persistant(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = ammo.begin(); it != ammo.end(); ++it)
            os << "ps_ammo(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = powerups.begin(); it != powerups.end(); ++it)
            os << "ps_powerups(" << it->first << "): " << it->second << " ";
        os << std::endl;
    }
}

bool PilotState::isAtributeFloat(int id) const {
    return (PilotNetfield[id].type == FIELD_FLOAT);
}

bool PilotState::isAtributeInteger(int id) const {
    return !isAtributeFloat(id);
}

bool PilotState::hasVehicleSet() const {
    return isAtributeSet(31); //m_iVehicleNum
}

void VehicleState::report(std::ostream& os) const {
    os << "    ";
    for (std::map<int, Atribute>::const_iterator it = atributes.begin();
        it != atributes.end(); ++it) {
        os << VehicleNetfield[it->first]._name << ": ";

        if ((VehicleNetfield[it->first].type == FIELD_FLOAT))
            os << it->second.fVal;
        else
            os << it->second.iVal;
        os << " ";
    }
    os << std::endl;

    if (!stats.empty() || !persistant.empty()
        || !ammo.empty() || !powerups.empty()) {
        os << "    ";
        for (statsarray_cit it = stats.begin(); it != stats.end(); ++it)
            os << "ps_stats(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = persistant.begin();
            it != persistant.end(); ++it)
            os << "ps_persistant(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = ammo.begin(); it != ammo.end(); ++it)
            os << "ps_ammo(" << it->first << "): " << it->second << " ";
        for (statsarray_cit it = powerups.begin(); it != powerups.end(); ++it)
            os << "ps_powerups(" << it->first << "): " << it->second << " ";
        os << std::endl;
    }
}

void VehicleState::save() const {
    //last changed byte
    if (!atributes.empty())
        Message::buffer.writeBits(((atributes.rbegin())->first) + 1, SIZE_8BITS);
    else
        Message::buffer.writeBits(0, SIZE_8BITS);

    int nulled = 0;
    int bufiVal; float buffVal;

    for (std::map<int, Atribute>::const_iterator it = atributes.begin();
        it != atributes.end(); ++it) {
        //null previous atributes
        for (; nulled != it->first; ++nulled) Message::buffer.writeBits(0, SIZE_1BIT);

        //here comes change
        Message::buffer.writeBits(1, SIZE_1BIT);

        bufiVal = it->second.iVal;
        buffVal = it->second.fVal;

        if (VehicleNetfield[it->first].type == FIELD_FLOAT) {
            //float number
            int buffValtrunc = (int)buffVal;

            if ((buffValtrunc == buffVal) && (buffValtrunc + FLOAT_INT_BIAS >= 0)
                && buffValtrunc + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS)) {
                Message::buffer.writeBits(0, SIZE_1BIT);
                Message::buffer.writeBits(buffValtrunc + FLOAT_INT_BIAS, SIZE_FLOATINT);
            }
            else {
                Message::buffer.writeBits(1, SIZE_1BIT);
                Message::buffer.writeBits(bufiVal, SIZE_32BITS);
            }

        }
        else {
            //integer
            Message::buffer.writeBits(bufiVal, VehicleNetfield[it->first].type);
        }

        ++nulled;
    }

    if (!stats.empty() || !persistant.empty()
        || !ammo.empty() || !powerups.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
        return;
    }

    int bits; statsarray_cit it;

    if (!stats.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = stats.begin(); it != stats.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = stats.begin(); it != stats.end(); ++it) {
            if (it->first == 4) Message::buffer.writeBits(it->second, SIZE_19BITS);
            else Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }

    if (!persistant.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = persistant.begin(); it != persistant.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = persistant.begin(); it != persistant.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }


    if (!ammo.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = ammo.begin(); it != ammo.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = ammo.begin(); it != ammo.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_16BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }

    if (!powerups.empty()) {
        Message::buffer.writeBits(1, SIZE_1BIT);

        bits = 0;
        for (it = powerups.begin(); it != powerups.end(); ++it)
            bits |= 1 << (it->first);
        Message::buffer.writeBits(bits, SIZE_16BITS);
        for (it = powerups.begin(); it != powerups.end(); ++it) {
            Message::buffer.writeBits(it->second, SIZE_32BITS);
        }
    }
    else {
        Message::buffer.writeBits(0, SIZE_1BIT);
    }
}

void VehicleState::load() {
    clear();

    //first byte gives upper bound of changed stats
    int lastchanged = Message::buffer.readBits(SIZE_8BITS);

    int size = sizeof(VehicleNetfield) / sizeof(Field);

    //now we read atributes one by one
    for (int i = 0; i < lastchanged; i++) {
        if (i < 0 || i >= size)
            throw DemoException("vehiclestate index out of range");
        //throw "vehiclestate index out of range";

        if (Message::buffer.readBits(1)) { //something changed here
            if (VehicleNetfield[i].type == FIELD_FLOAT) {
                //float number
                if (!Message::buffer.readBits(SIZE_1BIT)) {
                    //integral float
                    atributes[i].fVal = (float)Message::buffer.readBits(FLOAT_INT_BITS);
                    atributes[i].fVal -= FLOAT_INT_BIAS;
                }
                else {
                    //full floating point
                    atributes[i].iVal = Message::buffer.readBits(SIZE_32BITS);
                }
            }
            else {
                //integer
                atributes[i].iVal = Message::buffer.readBits(VehicleNetfield[i].type);
            }
        }
    }

    //nacitani statsu
    if (Message::buffer.readBits(SIZE_1BIT)) {
        int bits;

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) {
                    stats[i] = (i == 4) ? Message::buffer.readBits(SIZE_19BITS) : Message::buffer.readBits(SIZE_16BITS);
                }
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) {
                    persistant[i] = Message::buffer.readBits(SIZE_16BITS);
                }
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) {
                    ammo[i] = Message::buffer.readBits(SIZE_16BITS);
                }
        }

        if (Message::buffer.readBits(SIZE_1BIT)) {
            bits = Message::buffer.readBits(SIZE_16BITS);

            for (int i = 0; i < 16; i++)
                if (bits & (1 << i)) {
                    powerups[i] = Message::buffer.readBits(SIZE_32BITS);
                }
        }
    }
}

bool VehicleState::isAtributeFloat(int id) const {
    return (VehicleNetfield[id].type == FIELD_FLOAT);
}

bool VehicleState::isAtributeInteger(int id) const {
    return !isAtributeFloat(id);
}


DEMO_NAMESPACE_END