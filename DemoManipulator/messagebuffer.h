#ifndef MESSAGEBUFFER_H
#define MESSAGEBUFFER_H

#include "defs.h"
#include "message.h"

DEMO_NAMESPACE_START

class MessageBuffer {
    friend class Message;

private:
    //private Huffman class, only MessageBuffer uses it,
    //so no one outside needs to know it
    class Huffman;

private:
    byte    buffer[MAX_MSGLEN];
    int     currentPosition;
    int     length;

    static  Huffman huffman;

public:
    MessageBuffer();

    void clean();
    void load(std::ifstream& source, int len);
    void save(std::ofstream& dest);

    int readBits(int bitSize);
    std::string readString(bool big);

    void writeBits(int value, int bitSize);
    void writeString(const std::string& s, bool big);

    static void initHuffman();
};

class MessageBuffer::Huffman {

private:
    struct Node {
        struct Node* left, * right, * parent; /* tree structure */
        struct Node* next, * prev; /* doubly-linked list */
        struct Node** head; /* highest ranked node in block */
        int weight;
        int symbol;
    };

    struct HuffmanManipulator {
        int blocNode;
        int blocPtrs;

        Node* tree;
        Node* lhead;
        Node* ltail;
        Node* loc[HMAX + 1];
        Node** freelist;

        Node nodeList[768];
        Node* nodePtrs[768];
    };

    bool initialized;

    HuffmanManipulator compressor;
    HuffmanManipulator decompressor;

    //for initalizing and other private stuff
    void swap(HuffmanManipulator& huff, Node* node1, Node* node2);
    void swapList(Node* node1, Node* node2);
    void freePPNode(HuffmanManipulator& huff, Node** ppnode);
    void increment(HuffmanManipulator& huff, Node* node);
    Node** getPPNode(HuffmanManipulator& huff);
    void addRef(HuffmanManipulator& huff, byte ch);
    void send(MessageBuffer& msgbuff, Node* node, Node* child);

public:
    Huffman();

    void init();

    void putBit(MessageBuffer& msgbuff, char bit);
    void offsetTransmit(MessageBuffer& msgbuff, int ch);

    int getBit(MessageBuffer& msgbuff);
    int offsetReceive(MessageBuffer& msgbuff);

};

DEMO_NAMESPACE_END

#endif