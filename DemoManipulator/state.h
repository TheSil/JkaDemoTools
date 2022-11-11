#ifndef STATE_H
#define STATE_H

#include "defs.h"
#include "netfields.h"
#include "instruction.h"
#include "messagebuffer.h"

DEMO_NAMESPACE_START

typedef union{
		float		fVal; //32bit float
		int			iVal; //32bit int
} Atribute;

class MessageBuffer;

enum{
	INTEGER,
	FLOAT
};

enum STATE_TYPE {
	STATE_BASE,
	STATE_DELTAENTITY,
	STATE_PLAYERSTATE,
	STATE_PILOTSTATE,
	STATE_VEHICLESTATE
};

class PlayerState;

class State
{
friend class PlayerState;

protected:
typedef std::map<int,Atribute>					IntAtributeMap;
typedef std::map<int,Atribute>::iterator		IntAtributeMapIt;
typedef std::map<int,Atribute>::const_iterator	IntAtributeMapCit;
protected:
	int				type;
	IntAtributeMap	atributes;
public:
	State(int type) : type(type) {};
	virtual			~State() {};

	//clone
	virtual State*  clone() = 0;

	//I/O methods
	virtual void	report(std::ostream& os) const = 0;
	virtual void	save() const = 0;
	virtual	void	load() = 0;

	//get methods
	int				getType() const {return type;}; 

	float			getAtributeFloat(int id) const;
	int				getAtributeInt(int id) const;

	virtual	bool	isAtributeFloat(int id) const = 0;
	virtual bool	isAtributeInteger(int id) const = 0;

	int				getAtributesCount();

	//overloaded methods for setting attributes
	void			setAtribute(int id, float value);
	void			setAtribute(int id, int value);

	//other const methods
	bool			isAtributeSet(int id) const;

	//means that user changed something in objects
	virtual bool	isChanged() const = 0;

	//means that nothing is happening in this message
	virtual bool	noChanged() const = 0;

	//virtual void	update(PlayerState*) {};

	//removes all 0 in states
	virtual void	removeNull() {};

	virtual	void	clear();

	PlayerState*	getPlayerstate();

};

class EntityState : public State
{
protected:
	bool	toRemove;
	bool	previousToRemove;

public:
	EntityState() : State(STATE_DELTAENTITY),toRemove(false) , previousToRemove(false) {}; 
	~EntityState() {};

	State*  clone();

	//I/O methods
	void	report(std::ostream& os) const;
	void	save() const;
	void	load();

	//get methods
	bool	isAtributeFloat(int id) const;
	bool	isAtributeInteger(int id) const;


	//set methods
	void	setRemove(bool remove) {toRemove = remove;};

	//other const methods
	bool	isRemoved() const {return toRemove;};

	//means that user changed something in the object
	bool	isChanged() const;

	//means that nothing is happening in this message
	bool	noChanged() const;

	void	delta(const EntityState* state);
	void	applyOn(const EntityState* state);
	void	removeNull();

	void	clear();

	void	setprevtoremove(bool set) {previousToRemove = set;};
	bool	getprevtoremove() const {return previousToRemove;};
};

class PlayerState : public State
{
protected:
	typedef		std::map<int,int>					statsarray;
	typedef		std::map<int,int>::iterator			statsarray_it;
	typedef		std::map<int,int>::const_iterator	statsarray_cit;
protected:
	//additional attributes
	statsarray	stats;
	statsarray	persistant;
	statsarray	ammo;
	statsarray	powerups;

	PlayerState(int id) : State(id) {};

public:
	PlayerState() : State(STATE_PLAYERSTATE) {};
	
	virtual PlayerState*  clone();

	virtual ~PlayerState() {};

	void	report(std::ostream& os) const;
	void	save() const;
	void	load();
	bool	isChanged() const;
	bool	noChanged() const;

	virtual bool	hasVehicleSet() const;

	bool	isAtributeFloat(int id) const;
	bool	isAtributeInteger(int id) const;

	void	removeNull();
	void	delta(const PlayerState* state, bool isUncompressed);
	void	applyOn(PlayerState* state);

	void	clear();
};

class PilotState : public PlayerState
{
protected:

public:
	PilotState() : PlayerState(STATE_PILOTSTATE) {};
	~PilotState() {};

	//virtual PlayerState*  clone();

	void	report(std::ostream& os) const;
	void	save() const;
	void	load();

	virtual bool	hasVehicleSet() const;

	bool	isAtributeFloat(int id) const;
	bool	isAtributeInteger(int id) const;
};

class VehicleState : public PlayerState
{
public:

	VehicleState() : PlayerState(STATE_VEHICLESTATE) {};
	~VehicleState() {};

	//virtual PlayerState*  clone();

	void	report(std::ostream& os) const;
	void	save() const;
	void	load();

	bool	isAtributeFloat(int id) const;
	bool	isAtributeInteger(int id) const;
};

DEMO_NAMESPACE_END



#endif