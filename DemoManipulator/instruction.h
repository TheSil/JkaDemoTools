#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "defs.h"
#include "state.h"
#include "messagebuffer.h"

DEMO_NAMESPACE_START

class State;
class EntityState;

enum InstrTypes{
	INSTR_BASE = 0,
	INSTR_MAPCHANGE,
	INSTR_SERVERCOMMAND,
	INSTR_SNAPSHOT,
	INSTR_GAMESTATE,
};

class Gamestate;
class MapChange;
class Snapshot;
class ServerCommand;

class Instruction
{
protected:
	int		type;

	typedef	std::map<int,EntityState>					entitymap;
	typedef	std::map<int,EntityState>::iterator			entitymap_it;
	typedef	std::map<int,EntityState>::const_iterator	entitymap_cit;

public:
	Instruction(int type = INSTR_BASE) : type(type) {};
	virtual	~Instruction() {};

	//I/O methods
	virtual void			Save(/*MessageBuffer* msgbuffer*/)  const;
	virtual void			Load(/*MessageBuffer* msgbuffer*/);
	virtual void			report(std::ostream& os) const;		
	
	//get methods
	int						getType() const {return type;};

	//other methods

	//convert methods
	//BAD IDEA, need to modify base class for every new instruction type
	//TODO: remove and make clients use dynamic_cast instead
	MapChange*				getMapChange();
	Gamestate*				getGamestate();
	Snapshot*				getSnapshot();
	ServerCommand*			getServerCommand();


};

class MapChange : public Instruction
{
private:

public:
	MapChange() : Instruction(INSTR_MAPCHANGE) {};

	//I/O methods
	void	Save(/*MessageBuffer* msgbuffer*/) const;
	void	report(std::ostream& os) const;
};

class ServerCommand : public Instruction
{
private:
	int				sequenceNumber;
	std::string		command;

public:
	ServerCommand() : Instruction(INSTR_SERVERCOMMAND) {};

	//I/O methods
	void		Save(/*MessageBuffer* msgbuffer*/) const;
	void		Load(/*MessageBuffer* msgbuffer*/);
	void		report(std::ostream& os) const;

	int			getSequenceNumber() const {return sequenceNumber;};
	std::string getCommand() const {return command;};
};

class PlayerState;
class VehicleState;

class Snapshot : public Instruction
{
protected:
	int							serverTime;
	int							deltaNum;
	int							flags;
	std::vector<byte>			areaMask;

	PlayerState*				playerState;
	PlayerState*				vehicleState;
	entitymap					entities;
	
public:
	Snapshot(): Instruction(INSTR_SNAPSHOT), playerState(0), vehicleState(0) {};
	~Snapshot();

	//clone
	Snapshot*   clone();

	//I/O methods
	void		Save(/*MessageBuffer* msgbuffer*/) const;
	void		Load(/*MessageBuffer* msgbuffer*/);
	void		report(std::ostream& os) const;

	//get methods
	int				getAreamaskLen() const {return areaMask.size();};
	int				getAreamask(int id) const {return (int)areaMask[id];};
	int				getDeltanum() const {return deltaNum;};
	int				getServertime() const {return serverTime;};
	int				getSnapflags() const {return flags;};
	PlayerState*	getPlayerstate() {return playerState;};
	PlayerState*	getVehiclestate() {return vehicleState;};

	//set methods
	void		setAreamask(int id,int value) {areaMask[id] = value;};
	void		setAreamaskLen(int value) {areaMask.resize(value);};
	void		setSnapflags(int value) {flags = value;};
	void		setDeltanum(int value) {deltaNum = value;};
	void		setServertime(int value) {serverTime = value;};

	//if this is uncompressed snapshot, use this to remove all 0 values
	//TODO: maybe private methods?
	void		makeInit();
	void		removeNotChanged();

	//cutting comands
	//BAD, move this to Cutter class etc
	void		applyOn(Snapshot* snap);
	void		delta(Snapshot* snap);

	//hack like, only for purpose of optimizer
	//BAD
	entitymap&	getEntities() {return entities;};
};

class Gamestate : public Instruction
{
	struct MagicData{

		MagicData() : byte1(0), byte2(0), int1(0), int2(0) {};

		int	byte1, byte2;
		int int1, int2;
	};

protected:
	typedef	std::map<int,std::string>					stringmap;
	typedef	std::map<int,std::string>::iterator			stringmap_it;
	typedef	std::map<int,std::string>::const_iterator	stringmap_cit;

	int							commandSequence;
	int							clientNumber;
	int							checksumFeed;

	std::string					magicStuff;
	int							magicSeed;
	std::vector<MagicData>		magicData;

	/*
	int							magicNumber1;
	int							magicNumber2;
	*/

	entitymap					baseEntities;
	stringmap					configStrings;


public:	
	Gamestate() : Instruction(INSTR_GAMESTATE),
	  commandSequence(0), clientNumber(0), 
	  checksumFeed(0), magicSeed(0){};

	//I/O methods
	void			Save(/*MessageBuffer* msgbuffer*/) const;
	void			Load(/*MessageBuffer* msgbuffer*/);
	void			report(std::ostream& os) const;

	//get methods
	std::string		getConfigstring(int id);
	std::string		getMagicStuff();
	int				getMagicSeed();
	int				getMagicDataCount();
	void			getMagicData(unsigned id, int* byte1, int* byte2, int* int1, int* int2);

	//set methods
	void			setConfigstring(int id, const std::string& s);
	void			setMagicStuff(const std::string& s);
	void			setMagicSeed(int seed);
	void			setMagicData(unsigned id, int byte1, int byte2, int int1, int int2);

	void			removeConfigstring(int id);

	//load new things from server message
	void			update(const ServerCommand* servercommand);


};

DEMO_NAMESPACE_END

#endif