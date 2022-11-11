#include "demo.h"
#include <iostream>
#include <fstream>

using namespace DemoJKA;
using namespace std;

ofstream outputFile;

enum {
    MOD_RAW_TEXT,
    MOD_COLORED_TEXT,
    MOD_HTML
};

static const char* htmlcolors[] =
{
    "\"#000000\"", //0
    "\"#FF0000\"", //1
    "\"#00FF00\"", //2
    "\"#FFFF00\"", //3
    "\"#0000FF\"", //4
    "\"#00FFFF\"", //5
    "\"#FF00FF\"", //6
    "\"#FFFFFF\"", //7
};

string sanitize(const string& command, int mod) {
    //dest <= souce always (we only remove from source, never add)
    static char buffer[2048];
    int sourceId = 0;
    char* dest = buffer;

    for (; command[sourceId] != 0; ++sourceId)
    {
        if ((command[sourceId] == '^' && (command[sourceId + 1] >= '0') && (command[sourceId + 1] <= '7')) &&
            (mod == MOD_HTML || mod == MOD_RAW_TEXT)) {

            if (mod == MOD_HTML) {
                strcpy_s(dest, 40, "</font><font color=");
                dest += strlen("</font><font color=");

                strcpy_s(dest, 20, htmlcolors[command[sourceId + 1] - '0']);
                dest += strlen(htmlcolors[command[sourceId + 1] - '0']);

                *dest = '>';
                ++dest;
            }

            //need to increment here, two increments must be done
            ++sourceId;

        }
        else if (command[sourceId] == 25 || command[sourceId] == '"') {
            continue;
        }
        else {
            *dest = command[sourceId];
            ++dest;
        }
    }
    *dest = 0;

    return string(buffer);
}

void logCommand(const string& command, int mod) {
    const char* str = command.c_str();
    string chatLine;

    if (!strncmp(str, "chat", 4)) {
        chatLine = command.substr(5);
    }
    else if (!strncmp(str, "tchat", 5)) {
        chatLine = command.substr(6);
    }
    else {
        return;
    }

    chatLine = sanitize(chatLine, mod);

    if (mod == MOD_HTML) {
        outputFile << "<font color=#FFFFFF>";
    }

    outputFile << chatLine;

    if (mod == MOD_HTML) {
        outputFile << "</font><br>";
    }

    outputFile << endl;
}

int main(int argc, char** argv) {

    //-html
    //-text
    //-colored
    int mod = MOD_RAW_TEXT;

    if (argc < 3) {
        cout << "Need more arguments." << endl;

        cout << "Usage: DemoExtractor [input] [output] (parameter)" << endl;
        cout << "   input - input demo file, full name (with .dm_26 extension)" << endl;
        cout << "   output - output text/html file, full name (with .htm, .txt or w/e extension)" << endl;
        cout << "   parameter - can be one of following:" << endl;
        cout << "               -text - normal text will be generated (default)" << endl;
        cout << "               -html - special html file with colors will be generated" << endl;
        cout << "               -code - normal text WITH color codes will be generated" << endl;
        return 1;
    }

    if (argc >= 4) { //parameters
        if (!strncmp(argv[3], "-html", 5)) {
            mod = MOD_HTML;
        }
        else if (!strncmp(argv[3], "-text", 5)) {
            mod = MOD_RAW_TEXT;
        }
        else if (!strncmp(argv[3], "-code", 5)) {
            mod = MOD_COLORED_TEXT;
        }
    }

    Demo  inputDemo;
    outputFile.open(argv[2]);

    if (!inputDemo.open(argv[1])) {
        cout << "Input demo could not be opened." << endl;
        return 1;
    }

    if (outputFile.fail()) {
        cout << "Output file could not be opened." << endl;
        return 1;
    }

    int count = inputDemo.getMessageCount();
    int lastSeqNumber = 0;

    if (mod == MOD_HTML) {
        outputFile << "<html>\n<head></head>\n<body bgcolor=gray>\n<b>";
    }

    for (int i = 0; i < count; ++i) {
        Message* message = inputDemo.getMessage(i);

        if (!message)
            continue;

        for (int j = 0; j < message->getInstructionsCount(); ++j) {
            Instruction* instruction = message->getInstruction(j);
            ServerCommand* serverCommand = instruction->getServerCommand();
            if (serverCommand) {
                int seqNumber = serverCommand->getSequenceNumber();

                if (seqNumber <= lastSeqNumber) {
                    continue;
                }

                lastSeqNumber = seqNumber;

                logCommand(serverCommand->getCommand(), mod);
            }
        }

        inputDemo.unloadMessage(i);
    }

    if (mod == MOD_HTML) {
        outputFile << "\n</b>\n</body>\n</html>";
    }

    inputDemo.close();
    outputFile.close();

    return 0;
}