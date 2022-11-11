#include "messagebuffer.h"
/*

Implementantion of Huffman class which
is defined inside MessageBuffer class.

*/

DEMO_NAMESPACE_START

MessageBuffer::Huffman MessageBuffer::huffman;

int msgHashData[256] = {
250315,         // 0
41193,          // 1
6292,           // 2
7106,           // 3
3730,           // 4
3750,           // 5
6110,           // 6
23283,          // 7
33317,          // 8
6950,           // 9
7838,           // 10
9714,           // 11
9257,           // 12
17259,          // 13
3949,           // 14
1778,           // 15
8288,           // 16
1604,           // 17
1590,           // 18
1663,           // 19
1100,           // 20
1213,           // 21
1238,           // 22
1134,           // 23
1749,           // 24
1059,           // 25
1246,           // 26
1149,           // 27
1273,           // 28
4486,           // 29
2805,           // 30
3472,           // 31
21819,          // 32
1159,           // 33
1670,           // 34
1066,           // 35
1043,           // 36
1012,           // 37
1053,           // 38
1070,           // 39
1726,           // 40
888,            // 41
1180,           // 42
850,            // 43
960,            // 44
780,            // 45
1752,           // 46
3296,           // 47
10630,          // 48
4514,           // 49
5881,           // 50
2685,           // 51
4650,           // 52
3837,           // 53
2093,           // 54
1867,           // 55
2584,           // 56
1949,           // 57
1972,           // 58
940,            // 59
1134,           // 60
1788,           // 61
1670,           // 62
1206,           // 63
5719,           // 64
6128,           // 65
7222,           // 66
6654,           // 67
3710,           // 68
3795,           // 69
1492,           // 70
1524,           // 71
2215,           // 72
1140,           // 73
1355,           // 74
971,            // 75
2180,           // 76
1248,           // 77
1328,           // 78
1195,           // 79
1770,           // 80
1078,           // 81
1264,           // 82
1266,           // 83
1168,           // 84
965,            // 85
1155,           // 86
1186,           // 87
1347,           // 88
1228,           // 89
1529,           // 90
1600,           // 91
2617,           // 92
2048,           // 93
2546,           // 94
3275,           // 95
2410,           // 96
3585,           // 97
2504,           // 98
2800,           // 99
2675,           // 100
6146,           // 101
3663,           // 102
2840,           // 103
14253,          // 104
3164,           // 105
2221,           // 106
1687,           // 107
3208,           // 108
2739,           // 109
3512,           // 110
4796,           // 111
4091,           // 112
3515,           // 113
5288,           // 114
4016,           // 115
7937,           // 116
6031,           // 117
5360,           // 118
3924,           // 119
4892,           // 120
3743,           // 121
4566,           // 122
4807,           // 123
5852,           // 124
6400,           // 125
6225,           // 126
8291,           // 127
23243,          // 128
7838,           // 129
7073,           // 130
8935,           // 131
5437,           // 132
4483,           // 133
3641,           // 134
5256,           // 135
5312,           // 136
5328,           // 137
5370,           // 138
3492,           // 139
2458,           // 140
1694,           // 141
1821,           // 142
2121,           // 143
1916,           // 144
1149,           // 145
1516,           // 146
1367,           // 147
1236,           // 148
1029,           // 149
1258,           // 150
1104,           // 151
1245,           // 152
1006,           // 153
1149,           // 154
1025,           // 155
1241,           // 156
952,            // 157
1287,           // 158
997,            // 159
1713,           // 160
1009,           // 161
1187,           // 162
879,            // 163
1099,           // 164
929,            // 165
1078,           // 166
951,            // 167
1656,           // 168
930,            // 169
1153,           // 170
1030,           // 171
1262,           // 172
1062,           // 173
1214,           // 174
1060,           // 175
1621,           // 176
930,            // 177
1106,           // 178
912,            // 179
1034,           // 180
892,            // 181
1158,           // 182
990,            // 183
1175,           // 184
850,            // 185
1121,           // 186
903,            // 187
1087,           // 188
920,            // 189
1144,           // 190
1056,           // 191
3462,           // 192
2240,           // 193
4397,           // 194
12136,          // 195
7758,           // 196
1345,           // 197
1307,           // 198
3278,           // 199
1950,           // 200
886,            // 201
1023,           // 202
1112,           // 203
1077,           // 204
1042,           // 205
1061,           // 206
1071,           // 207
1484,           // 208
1001,           // 209
1096,           // 210
915,            // 211
1052,           // 212
995,            // 213
1070,           // 214
876,            // 215
1111,           // 216
851,            // 217
1059,           // 218
805,            // 219
1112,           // 220
923,            // 221
1103,           // 222
817,            // 223
1899,           // 224
1872,           // 225
976,            // 226
841,            // 227
1127,           // 228
956,            // 229
1159,           // 230
950,            // 231
7791,           // 232
954,            // 233
1289,           // 234
933,            // 235
1127,           // 236
3207,           // 237
1020,           // 238
927,            // 239
1355,           // 240
768,            // 241
1040,           // 242
745,            // 243
952,            // 244
805,            // 245
1073,           // 246
740,            // 247
1013,           // 248
805,            // 249
1008,           // 250
796,            // 251
996,            // 252
1057,           // 253
11457,          // 254
13504,          // 255
};

MessageBuffer::Huffman::Node** MessageBuffer::Huffman::getPPNode(HuffmanManipulator& huff) {
    Node** tppnode;
    if (!huff.freelist) {
        return &(huff.nodePtrs[huff.blocPtrs++]);
    }
    else {
        tppnode = huff.freelist;
        huff.freelist = (Node**)*tppnode;
        return tppnode;
    }
}

void MessageBuffer::Huffman::swap(HuffmanManipulator& huff, Node* node1, Node* node2) {
    Node* par1, * par2;

    par1 = node1->parent;
    par2 = node2->parent;

    if (par1) {
        if (par1->left == node1) {
            par1->left = node2;
        }
        else {
            par1->right = node2;
        }
    }
    else {
        huff.tree = node2;
    }

    if (par2) {
        if (par2->left == node2) {
            par2->left = node1;
        }
        else {
            par2->right = node1;
        }
    }
    else {
        huff.tree = node1;
    }

    node1->parent = par2;
    node2->parent = par1;
}

void MessageBuffer::Huffman::swapList(Node* node1, Node* node2) {
    Node* par1;

    par1 = node1->next;
    node1->next = node2->next;
    node2->next = par1;

    par1 = node1->prev;
    node1->prev = node2->prev;
    node2->prev = par1;

    if (node1->next == node1) {
        node1->next = node2;
    }
    if (node2->next == node2) {
        node2->next = node1;
    }
    if (node1->next) {
        node1->next->prev = node1;
    }
    if (node2->next) {
        node2->next->prev = node2;
    }
    if (node1->prev) {
        node1->prev->next = node1;
    }
    if (node2->prev) {
        node2->prev->next = node2;
    }
}

void MessageBuffer::Huffman::freePPNode(HuffmanManipulator& huff, Node** ppnode) {
    *ppnode = (Node*)huff.freelist;
    huff.freelist = ppnode;
}

void MessageBuffer::Huffman::increment(HuffmanManipulator& huff, Node* node) {
    Node* lnode;

    if (!node) {
        return;
    }

    if (node->next != NULL && node->next->weight == node->weight) {
        lnode = *node->head;
        if (lnode != node->parent) {
            swap(huff, lnode, node);
        }
        swapList(lnode, node);
    }
    if (node->prev && node->prev->weight == node->weight) {
        *node->head = node->prev;
    }
    else {
        *node->head = NULL;
        freePPNode(huff, node->head);
    }
    node->weight++;
    if (node->next && node->next->weight == node->weight) {
        node->head = node->next->head;
    }
    else {
        node->head = getPPNode(huff);
        *node->head = node;
    }
    if (node->parent) {
        increment(huff, node->parent);
        if (node->prev == node->parent) {
            swapList(node, node->parent);
            if (*node->head == node) {
                *node->head = node->parent;
            }
        }
    }
}

void MessageBuffer::Huffman::addRef(HuffmanManipulator& huff, byte ch) {
    Node* tnode, * tnode2;
    if (huff.loc[ch] == 0) { /* if this is the first transmission of this node */
        tnode = &(huff.nodeList[huff.blocNode++]);
        tnode2 = &(huff.nodeList[huff.blocNode++]);

        tnode2->symbol = INTERNAL_NODE;
        tnode2->weight = 1;
        tnode2->next = huff.lhead->next;
        if (huff.lhead->next) {
            huff.lhead->next->prev = tnode2;
            if (huff.lhead->next->weight == 1) {
                tnode2->head = huff.lhead->next->head;
            }
            else {
                tnode2->head = getPPNode(huff);
                *tnode2->head = tnode2;
            }
        }
        else {
            tnode2->head = getPPNode(huff);
            *tnode2->head = tnode2;
        }
        huff.lhead->next = tnode2;
        tnode2->prev = huff.lhead;

        tnode->symbol = ch;
        tnode->weight = 1;
        tnode->next = huff.lhead->next;
        if (huff.lhead->next) {
            huff.lhead->next->prev = tnode;
            if (huff.lhead->next->weight == 1) {
                tnode->head = huff.lhead->next->head;
            }
            else {
                /* this should never happen */
                tnode->head = getPPNode(huff);
                *tnode->head = tnode2;
            }
        }
        else {
            /* this should never happen */
            tnode->head = getPPNode(huff);
            *tnode->head = tnode;
        }
        huff.lhead->next = tnode;
        tnode->prev = huff.lhead;
        tnode->left = tnode->right = 0;

        if (huff.lhead->parent) {
            if (huff.lhead->parent->left == huff.lhead) { /* lhead is guaranteed to by the NYT */
                huff.lhead->parent->left = tnode2;
            }
            else {
                huff.lhead->parent->right = tnode2;
            }
        }
        else {
            huff.tree = tnode2;
        }

        tnode2->right = tnode;
        tnode2->left = huff.lhead;

        tnode2->parent = huff.lhead->parent;
        huff.lhead->parent = tnode->parent = tnode2;

        huff.loc[ch] = tnode;

        increment(huff, tnode2->parent);
    }
    else {
        increment(huff, huff.loc[ch]);
    }

}

MessageBuffer::Huffman::Huffman() : initialized(false) {
    memset(&compressor, 0, sizeof(HuffmanManipulator));
    memset(&decompressor, 0, sizeof(HuffmanManipulator));
}

void MessageBuffer::Huffman::init() {
    if (initialized) //already initialized
        return;

    // Initialize the tree & list with the NYT node 
    decompressor.tree = decompressor.lhead = decompressor.ltail = decompressor.loc[NYT] = &(decompressor.nodeList[decompressor.blocNode++]);
    decompressor.tree->symbol = NYT;
    decompressor.tree->weight = 0;
    decompressor.lhead->next = decompressor.lhead->prev = NULL;
    decompressor.tree->parent = decompressor.tree->left = decompressor.tree->right = NULL;

    // Add the NYT (not yet transmitted) node into the tree/list */
    compressor.tree = compressor.lhead = compressor.loc[NYT] = &(compressor.nodeList[compressor.blocNode++]);
    compressor.tree->symbol = NYT;
    compressor.tree->weight = 0;
    compressor.lhead->next = compressor.lhead->prev = NULL;
    compressor.tree->parent = compressor.tree->left = compressor.tree->right = NULL;
    compressor.loc[NYT] = compressor.tree;

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < msgHashData[i]; j++) {
            addRef(compressor, (byte)i);    // Do update
            addRef(decompressor, (byte)i);  // Do update
        }
    }

    initialized = true;
}

void MessageBuffer::Huffman::putBit(MessageBuffer& msgbuff, char bit) {
    init();

    if ((msgbuff.currentPosition & 7) == 0) { //init bit setting i guess
        msgbuff.buffer[(msgbuff.currentPosition >> 3)] = 0;
    }

    msgbuff.buffer[(msgbuff.currentPosition >> 3)] |= (bit << (msgbuff.currentPosition & 7));

    ++msgbuff.currentPosition;
}

void MessageBuffer::Huffman::send(MessageBuffer& msgbuff, Node* node, Node* child) {
    if (node->parent) {
        send(msgbuff, node->parent, node);
    }
    if (child) {
        if (node->right == child) {
            putBit(msgbuff, 1);
        }
        else {
            putBit(msgbuff, 0);
        }
    }
}

void MessageBuffer::Huffman::offsetTransmit(MessageBuffer& msgbuff, int ch) {
    init();

    send(msgbuff, compressor.loc[ch], 0);
}

int MessageBuffer::Huffman::getBit(MessageBuffer& msgbuff) {
    init();

    int t = (msgbuff.buffer[(msgbuff.currentPosition >> 3)]
        >> (msgbuff.currentPosition & 7)) & 0x1;
    ++msgbuff.currentPosition;

    return t;
}

int MessageBuffer::Huffman::offsetReceive(MessageBuffer& msgbuff) {
    init();

    int t;
    Node* node = decompressor.tree;

    while (node && node->symbol == INTERNAL_NODE) {
        t = (msgbuff.buffer[(msgbuff.currentPosition >> 3)] >> (msgbuff.currentPosition & 7)) & 0x1;
        ++msgbuff.currentPosition;

        if (t)
            node = node->right;
        else
            node = node->left;
    }

    if (!node) {
        return 0;
    }

    return node->symbol;
}

DEMO_NAMESPACE_END