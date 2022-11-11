#include "message.h"
#include "messagebuffer.h"
#include "defs.h"

DEMO_NAMESPACE_START

bool Message::forceVehicleLoad = false;
MessageBuffer Message::buffer;

class MessageImpl {
public:

    int  sequenceNumber;
    int  reliableAcknowledge;
    bool loaded;

    std::vector<Instruction*> instructions;
};

void Message::load(std::ifstream& is) {
    int msglen;

    is.read((char*)&(impl->sequenceNumber), sizeof(impl->sequenceNumber));
    is.read((char*)&(msglen), sizeof(msglen));

    if ((impl->sequenceNumber == -1 && msglen == -1) || is.fail()) //ending message
        return;

    //load data field to special buffer
    //which knows how to read it
    Message::buffer.load(is, msglen);

    try {

        impl->reliableAcknowledge = Message::buffer.readBits(SIZE_32BITS);

        int cmd;
        Instruction* tmpInstr;

        while (true) {
            cmd = Message::buffer.readBits(SIZE_8BITS); //byte command specifier

            if (cmd == svc_EOF)
                break;

            switch (cmd) {
            case svc_bad:
                break;
            case svc_nop:
                break;
            case svc_snapshot:
                tmpInstr = new Snapshot();
                tmpInstr->Load();
                impl->instructions.push_back(tmpInstr);
                break;
            case svc_serverCommand:
                tmpInstr = new ServerCommand();
                tmpInstr->Load();
                impl->instructions.push_back(tmpInstr);
                break;
            case svc_gamestate:
                tmpInstr = new Gamestate();
                tmpInstr->Load();
                impl->instructions.push_back(tmpInstr);
                break;
            case svc_mapchange:
                tmpInstr = new MapChange();
                impl->instructions.push_back(tmpInstr);
                break;
            default:
                throw DemoException("unknown message type");
                break;
            }
        }
    }
    catch (std::exception& e) {
        Message::buffer.clean();
        throw e;
    }

    impl->loaded = true; //successfully loaded
    Message::buffer.clean();
}

void Message::save(std::ofstream& os) const {
    Message::buffer.clean();

    Message::buffer.writeBits(impl->reliableAcknowledge, SIZE_32BITS);

    for (std::vector<Instruction*>::const_iterator it = impl->instructions.begin();
        it != impl->instructions.end(); ++it) {
        (*it)->Save();
    }
    Message::buffer.writeBits(svc_EOF, SIZE_8BITS);

    os.write((char*)&(impl->sequenceNumber), sizeof(impl->sequenceNumber));
    os.write((char*)&(Message::buffer.length), sizeof(Message::buffer.length));

    Message::buffer.save(os);
    Message::buffer.clean();
}

Message::Message() : impl(new MessageImpl()) {
    impl->loaded = false;
};

Message::~Message() {

    for (std::vector<Instruction*>::iterator it = impl->instructions.begin();
        it != impl->instructions.end(); ++it) {
        delete* it;
    }

    delete impl;
}

bool Message::isLoad() const {
    return impl->loaded;
};

void Message::setSeqNumber(int seq) {
    impl->sequenceNumber = seq;
};

void Message::setRelAcknowledge(int rel) {
    impl->reliableAcknowledge = rel;
};

int Message::getSeqNumber() const {
    return impl->sequenceNumber;
};

int Message::getRelAcknowledge() const {
    return impl->reliableAcknowledge;
};

Instruction* Message::getInstruction(int id) {
    assert((id >= 0) && (id < impl->instructions.size()));
    return impl->instructions[id];
};

int Message::getInstructionsCount() const {
    return (int)impl->instructions.size();
};

void Message::deleteInstruction(int id, int n) {
    if (!impl->loaded)
        return;

    assert((id >= 0) && (id < impl->instructions.size()));
    assert(n >= 0);

    for (int i = id; i < id + n; ++i)
        delete impl->instructions[i];

    impl->instructions.erase(impl->instructions.begin() + id,
        impl->instructions.begin() + id + n);
}

void Message::clear() {
    for (int i = 0; i < (int)impl->instructions.size(); ++i) {
        delete impl->instructions[i];
    }

    impl->instructions.clear();
}

bool Message::saveMessage(std::ofstream& os) const {
    if (!impl->loaded)
        return false;

    save(os);

    return true;
}

DEMO_NAMESPACE_END