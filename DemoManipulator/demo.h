#ifndef DEMO_H
#define DEMO_H

#include "message.h"

DEMO_NAMESPACE_START

enum{
	VEHICLE_NOT_CHECKED = 0,
	VEHICLE_INSIDE,
	VEHICLE_NOT_INSIDE
};

class DemoImpl;

class Demo
{
private:
	DemoImpl*	impl;

public:

	/**

	Constructor. 

	*/
	Demo();

	/**

	Destructor. Deallocates memory used by messages and closes
	demo file input.

	@see close()

	*/
	~Demo();

	/**

	Attempts to load demo file with given name. Initialize indexes
	to messages and optionaly can perform analysis.

	@param filename full path of demo file 
	@param analysis set to perform analysis right after loading
	@return true if demo was successfuly opened

	@see analyse()

	*/
	bool		open(const char* filename, bool analysis = true);

	/**

	Checks if whether this objects has demo loaded.

	@return true if demo is loaded, false otherwise

	*/
	bool		isOpen() const;

	/**

	Manually deallocates memory used by messages and closes
	demo file input. This is also done in destructor.

	@see ~Demo()
	@see unloadMessage()

	*/
	void		close(); 

	/**

	Saves current content of this object to demo file with given
	name. If file already exists, it is ovewritten. Output file
	respects standard JKA demo format.

	@param filename full path to output demo file
	@param endSign tells whether demo should end with specific message
	       containing two consecutive -1 (proper ending)
	@return false if opening output file fails for any reason, true otherwise

	*/
	bool		save(const char* filename, bool endSign = false) const;

	/**

	Loads message from input demo file, unless this message has been already loaded.
	In that case it does nothing.

	@param id id number of message

	@see getMessageCount()
	@see unloadMessage()

	*/
	void		loadMessage(int id);

	/**

	Check if message has been loaded to memory.

	@param id id number of message

	@return true if message has been loaded, false otherwise

	@see loadMessage()
	@see unloadMessage()

	*/
	bool		isMessageLoaded(int id) const;	

	/**

	Completely unloads message from memory, only indexing and
	analysis information are kept. Any changes did to this
	message will be discarded.

	@param id id number of message


	@see loadMessage()
	@see isMessageLoaded()

	*/
	void		unloadMessage(int id); 

	/**

	Perform special analysis. This check is needed to know about
	maps/restarts distribution in demo aswell as to be able
	correctly load messages containing vehicles.

	*/
	void        analyse();

	/**

	Returns pointer to selected message. If message isnt yet loaded,
	loadMessage() is called.

	@param id id number of message
	@return pointer to selected message if index is fine and message has
	        been sucessfuly loaded, otherwise returns null pointer

	@see loadMessage()

	*/
	Message*	getMessage(int id);

	/**
	Returns messages count.

	*/
	int			getMessageCount() const;

	//map offsets vector api

	/**

	Returns number of map changes and restarts. analyse() must be called
	before this.

	@return number of maps/restarts if analysis has been done, 0 otherwise

	@see analyse()

	*/
	int			getMapsCount() const;

	/**

	Gives name of selected map. analyse() must be called
	before this.

	@param mapId index of map
	@return name of selected map, in case of restart it is "restart"

	@see analyse()
	@see getMapsCount()

	*/
	std::string	getMapName(int mapId) const;

	/**

	Gives message index of first snapshot for selected map. analyse() 
	must be called before this.

	@param mapId index of map
	@return name of selected map

	@see analyse()
	@see getMapsCount()

	*/
	int			getMapId(int mapId) const;

	/**

	Checks whether selected map is new map change or just restart.
	analyse() must be called before this.

	@param mapId index of map
	@return true if given map is restart, false otherwise

	@see analyse()
	@see getMapsCount()

	*/
	bool		isMapRestart(int mapId) const;

	/**

	Gives initial time for selected map. This information can be normally
	found in configstring #21 in gamestate. analyse() must be called before 
	this.

	@param mapId index of map
	@return initial time of selected map in ms

	@see analyse()
	@see getMapsCount()

	*/
	int			getMapStartTime(int mapId) const;	

	/**

	Gives ending time for selected map. This information can be found
	on next map/restart or on demo end. analyse() must be called before 
	this.

	@param mapId index of map
	@return initial time of selected map in ms

	@see analyse()
	@see getMapsCount()

	*/
	int			getMapEndTime(int mapId) const;	

	/**

	Save selected message to output stream. Used for manually saving
	only desired messages. Use save() to save whole demo.

	@param id id number of message
	@param os output stream (file)

	*/
	void		saveMessage(int id, std::ofstream& os) const;

	/**

	Delete message(s) from memory aswell as all indexing or analysed info. This
	will corrupted demo format unless you correct all messages refering to this one.

	@param id index of first message to be deleted
	@param endid index of last message to be deleted, only matters when endid > id


	*/
	void		deleteMessage(int startid, int endid = 0); 

};

DEMO_NAMESPACE_END

#endif