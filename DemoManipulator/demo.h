#ifndef DEMO_H
#define DEMO_H

#include "message.h"

DEMO_NAMESPACE_START

enum {
    VEHICLE_NOT_CHECKED = 0,
    VEHICLE_INSIDE,
    VEHICLE_NOT_INSIDE
};

class DemoImpl;

class Demo
{
private:
    DemoImpl* impl;

public:

    Demo();
    ~Demo();

    /*
    Attempts to load demo file with given name. Initialize indexes
    to messages and optionaly can perform analysis.

    filename full path of demo file
    analysis - set to perform analysis right after loading
    */
    bool open(const char* filename, bool analysis = true);

    /*
    Checks if whether this objects has demo loaded.
    */
    bool isOpen() const;

    /*
    Manually deallocates memory used by messages and closes
    demo file input. This is also done in destructor.
    */
    void close();

    /*
    Saves current content of this object to demo file with given
    name. If file already exists, it is ovewritten. Output file
    respects standard JKA demo format.

    filename - full path to output demo file
    endSign - tells whether demo should end with specific message
           containing two consecutive -1 (proper ending)
    */
    bool save(const char* filename, bool endSign = false) const;

    /*
    Loads message from input demo file, unless this message has been already loaded.
    In that case it does nothing.
    */
    void loadMessage(int id);
    bool isMessageLoaded(int id) const;

    /*
    Completely unloads message from memory, only indexing and
    analysis information are kept. Any changes did to this
    message will be discarded.
    */
    void unloadMessage(int id);

    /*
    Perform special analysis. This check is needed to know about
    maps/restarts distribution in demo aswell as to be able
    correctly load messages containing vehicles.
    */
    void analyse();

    /*
    Returns pointer to selected message. If message isnt yet loaded,
    loadMessage() is called.
    */
    Message* getMessage(int id);

    int getMessageCount() const;

    /*
    Returns number of map changes and restarts. analyse() must be called
    before this.
    */
    int getMapsCount() const;

    /*
    Gives name of selected map. analyse() must be called
    before this.
    */
    std::string	getMapName(int mapId) const;

    /*
    Gives message index of first snapshot for selected map. analyse()
    must be called before this.
    */
    int getMapId(int mapId) const;

    /*
    Checks whether selected map is new map change or just restart.
    analyse() must be called before this.
    */
    bool isMapRestart(int mapId) const;

    /*
    Gives initial time for selected map. This information can be normally
    found in configstring #21 in gamestate. analyse() must be called before
    this.
    */
    int getMapStartTime(int mapId) const;

    /*
    Gives ending time for selected map. This information can be found
    on next map/restart or on demo end. analyse() must be called before
    this.
    */
    int getMapEndTime(int mapId) const;

    /*
    Save selected message to output stream. Used for manually saving
    only desired messages. Use save() to save whole demo.
    */
    void saveMessage(int id, std::ofstream& os) const;

    /*
    Delete message(s) from memory aswell as all indexing or analysed info. This
    will corrupted demo format unless you correct all messages refering to this one.

    id - index of first message to be deleted
    endid - index of last message to be deleted, only matters when endid > id
    */
    void deleteMessage(int startid, int endid = 0);

};

DEMO_NAMESPACE_END

#endif