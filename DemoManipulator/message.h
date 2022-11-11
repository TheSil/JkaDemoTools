#ifndef MESSAGE_H
#define MESSAGE_H

#include "instruction.h"

DEMO_NAMESPACE_START

class Instruction;

class MessageImpl;
class MessageBuffer;

class Message
{
    friend class Demo;
    friend class DemoImpl;

private:
    MessageImpl* impl;

    void load(std::ifstream& is);
    void save(std::ofstream& os) const;

public:

    //this is ugly, i should instead create
    //some loader class that will be able to
    //save/load all of instructions and states
    //still using shared buffer
    //Demo::Loader with access only from class Demo
    //and will be friend to both Instruction and State
    static MessageBuffer buffer;

    Message();
    ~Message();

    void clear();

    static bool forceVehicleLoad;

    bool isLoad() const;

    void setSeqNumber(int seq);
    void setRelAcknowledge(int rel);

    int getSeqNumber() const;
    int getRelAcknowledge() const;

    Instruction* getInstruction(int id);
    int getInstructionsCount() const;

    //delete instructions in range [id,endid)
    void deleteInstruction(int id, int n = 1);

    bool saveMessage(std::ofstream& os) const;
};

DEMO_NAMESPACE_END

#endif