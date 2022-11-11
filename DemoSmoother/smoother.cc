#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "..\DemoManipulator\demo.h"
#include <time.h>

using namespace DemoJKA;
using namespace std;

//ofstream logfile("output.log");

bool CheckArguments(int argCount){
	if (argCount < 2)
		return false;

	return true;
}

Snapshot*	getFirstSnapshot(Message* message){

	for(int i=0; i < message->getInstructionsCount(); ++i)
		if (message->getInstruction(i)->getType() == INSTR_SNAPSHOT)
			return message->getInstruction(i)->getSnapshot();

	return 0;
}

void clearServerCommands(Message* current){
	static map<int,int>	serverCommands;
	Instruction*	currentInstruction;

	for(int i=0; i < current->getInstructionsCount(); ++i){
		currentInstruction = current->getInstruction(i);

		if (currentInstruction->getType() == INSTR_SERVERCOMMAND) { 
			//get rid of duplicit server commands
			if (serverCommands.find(currentInstruction->getServerCommand()->getSequenceNumber())
					!= serverCommands.end() ){ //found already in received commands
				current->deleteInstruction(i);
				--i; //very important, so we can continue in for cycle without problems
				continue;
			} else {
				serverCommands[currentInstruction->getServerCommand()->getSequenceNumber()] = 0;
			}

		}

	}
}

static bool removedEntities[1024];
static bool notChanged[1024];

void removeNotChanged(Snapshot*	snap){
	//std::cout << "snapshot server time: " << serverTime << std::endl;

	for(std::map<int,EntityState>::iterator it = snap->getEntities().begin(); it!=snap->getEntities().end();){

		if (it->first < 32) { //player entities
			if ( it->second.noChanged() ){
				if ( !it->second.isRemoved() ){
					snap->getEntities().erase(it++);
				} else { //now should be removed
					if (removedEntities[it->first]) { //have been removed before
						snap->getEntities().erase(it++);
					} else {
						removedEntities[it->first] = true;
						++it;
					}

				}

			} else { //entity has some changes, clear "been removed" status
				removedEntities[it->first] = false;
				++it;
			}

		} else { //world entities

			if ( it->second.isRemoved() ){
				if (removedEntities[it->first]) { //have been removed before
					snap->getEntities().erase(it++);
				} else {
					removedEntities[it->first] = true;
					notChanged[it->first] = false;
					++it;
				}
			} else if ( it->second.noChanged() ){
				if (notChanged[it->first]) { //have been marked as "not changed" before
					snap->getEntities().erase(it++);
				} else {
					notChanged[it->first] = true;
					removedEntities[it->first] = false;
					++it;
				}
			} else {
				notChanged[it->first] = false;
				removedEntities[it->first] = false;
				++it;
			}

			



		}		
	

	}

}

const int EF_TELEPORT_BIT = 1<<3;
const int INDEX_EFLAGS = 17;
const int INDEX_BOBCYCLE = 9;
const int INDEX_ORIGIN0 = 2;
const int INDEX_ORIGIN1 = 1;
const int INDEX_ORIGIN2 = 5;
const int INDEX_VEL0 = 6;
const int INDEX_VEL1 = 7;
const int INDEX_VEL2 = 8;
const int INDEX_ANGLES0 = 4;
const int INDEX_ANGLES1 = 3;
const int INDEX_ANGLES2 = 50;

Demo demoToOptimize;
ofstream	ouputCopy;
Snapshot* lastSavedSnap = 0;
int lastSavedSeqNumber = 0;
int queueStartId = 0;
int interpolatedSoft = 0;

void Interpolate(Snapshot* what, Snapshot* from, Snapshot* to){

	//check for teleport
	if ( (from->getPlayerstate()->getAtributeInt(INDEX_EFLAGS) ^ to->getPlayerstate()->getAtributeInt(INDEX_EFLAGS) ) 
		& EF_TELEPORT_BIT )
		return;

	if (!what){ //nothing to interpolate
		return;
	}

	int toTime = to->getServertime();
	int fromTime = from->getServertime();

	//server times
	if (toTime <= fromTime)
		return;

	float f = (float)( what->getServertime() - fromTime ) / ( toTime - fromTime );

	
	//interpolate bobcycle
	int fromBobcycle, toBobcycle;

	fromBobcycle = from->getPlayerstate()->getAtributeInt(INDEX_BOBCYCLE);
	toBobcycle = to->getPlayerstate()->getAtributeInt(INDEX_BOBCYCLE);
	if ( toBobcycle < fromBobcycle ) {
		toBobcycle += 256;		// handle wraparound
	}

	what->getPlayerstate()->setAtribute(INDEX_BOBCYCLE, (int)(fromBobcycle + f * ( toBobcycle - fromBobcycle )) ); 
	

	//interpolate origin
	float fromOrigin[3],toOrigin[3];

	fromOrigin[0] = from->getPlayerstate()->getAtributeFloat(INDEX_ORIGIN0);
	fromOrigin[1] = from->getPlayerstate()->getAtributeFloat(INDEX_ORIGIN1);
	fromOrigin[2] = from->getPlayerstate()->getAtributeFloat(INDEX_ORIGIN2);
	toOrigin[0] = to->getPlayerstate()->getAtributeFloat(INDEX_ORIGIN0);
	toOrigin[1] = to->getPlayerstate()->getAtributeFloat(INDEX_ORIGIN1);
	toOrigin[2] = to->getPlayerstate()->getAtributeFloat(INDEX_ORIGIN2);

	what->getPlayerstate()->setAtribute(INDEX_ORIGIN0, fromOrigin[0] + f * (toOrigin[0] - fromOrigin[0] ) ); 
	what->getPlayerstate()->setAtribute(INDEX_ORIGIN1, fromOrigin[1] + f * (toOrigin[1] - fromOrigin[1] ) ); 
	what->getPlayerstate()->setAtribute(INDEX_ORIGIN2, fromOrigin[2] + f * (toOrigin[2] - fromOrigin[2] ) ); 

	//interpolate velocity
	float fromVelocity[3],toVelocity[3];

	fromVelocity[0] = from->getPlayerstate()->getAtributeFloat(INDEX_VEL0);
	fromVelocity[1] = from->getPlayerstate()->getAtributeFloat(INDEX_VEL1);
	fromVelocity[2] = from->getPlayerstate()->getAtributeFloat(INDEX_VEL2);
	toVelocity[0] = to->getPlayerstate()->getAtributeFloat(INDEX_VEL0);
	toVelocity[1] = to->getPlayerstate()->getAtributeFloat(INDEX_VEL1);
	toVelocity[2] = to->getPlayerstate()->getAtributeFloat(INDEX_VEL2);

	what->getPlayerstate()->setAtribute(INDEX_VEL0, fromVelocity[0] + f * (toVelocity[0] - fromVelocity[0] ) ); 
	what->getPlayerstate()->setAtribute(INDEX_VEL1, fromVelocity[1] + f * (toVelocity[1] - fromVelocity[1] ) ); 
	what->getPlayerstate()->setAtribute(INDEX_VEL2, fromVelocity[2] + f * (toVelocity[2] - fromVelocity[2] ) ); 

	//interpolate viewangles
	float fromAngles[3],toAngles[3];
	
	fromAngles[0] = from->getPlayerstate()->getAtributeFloat(INDEX_ANGLES0);
	fromAngles[1] = from->getPlayerstate()->getAtributeFloat(INDEX_ANGLES1);
	fromAngles[2] = from->getPlayerstate()->getAtributeFloat(INDEX_ANGLES2);
	toAngles[0] = to->getPlayerstate()->getAtributeFloat(INDEX_ANGLES0);
	toAngles[1] = to->getPlayerstate()->getAtributeFloat(INDEX_ANGLES1);
	toAngles[2] = to->getPlayerstate()->getAtributeFloat(INDEX_ANGLES2);

	for(int i=0;i<3;++i){
		if ( toAngles[i] - fromAngles[i] > 180 ) {
			toAngles[i] -= 360;
		}
		if ( toAngles[i] - fromAngles[i] < -180 ) {
			toAngles[i] += 360;
		}
	}

	what->getPlayerstate()->setAtribute(INDEX_ANGLES0, fromAngles[0] + f * (toAngles[0] - fromAngles[0] ) ); 
	what->getPlayerstate()->setAtribute(INDEX_ANGLES1, fromAngles[1] + f * (toAngles[1] - fromAngles[1] ) ); 
	what->getPlayerstate()->setAtribute(INDEX_ANGLES2, fromAngles[2] + f * (toAngles[2] - fromAngles[2] ) ); 

	//interpolate commandtime
	int fromCmdTime, toCmdTime;
	fromCmdTime = from->getPlayerstate()->getAtributeInt(0);
	toCmdTime = to->getPlayerstate()->getAtributeInt(0);

	what->getPlayerstate()->setAtribute(0, (int)(fromCmdTime + f * (toCmdTime - fromCmdTime)) ); 


}

void uncompressMessage(int messageId){
	Snapshot* currentSnap = getFirstSnapshot(demoToOptimize.getMessage(messageId));
	int seekingSeqNumber;
	int seekingMessageId;
	Message* seekingMessage;


	//dereferencing current snapshot so it is not compressed
	if (currentSnap && currentSnap->getDeltanum() != 0){
		seekingMessageId = messageId - currentSnap->getDeltanum();
		seekingSeqNumber = demoToOptimize.getMessage(messageId)->getSeqNumber() - currentSnap->getDeltanum();
		seekingMessage = demoToOptimize.getMessage(seekingMessageId);

		while (seekingMessage && seekingMessage->getSeqNumber() != seekingSeqNumber){
			if (seekingSeqNumber < seekingMessage->getSeqNumber())
				--seekingMessageId;
			else
				++seekingMessageId;

			seekingMessage = demoToOptimize.getMessage(seekingMessageId);
		}

		Snapshot* deltaSnap = getFirstSnapshot(seekingMessage);
		currentSnap->applyOn(deltaSnap);
		currentSnap->setDeltanum(0);
	}


}

static bool lastMessageWasGamestate = false;

void save(int messageId){
	Message*	message = demoToOptimize.getMessage(messageId);
	Snapshot*	snapshot = getFirstSnapshot(message);
	int commandTime;

	if (snapshot && lastSavedSnap && !lastMessageWasGamestate){
		Snapshot* tempLastSavedSnap = lastSavedSnap;
		lastSavedSnap = snapshot->clone();

		commandTime = snapshot->getPlayerstate()->getAtributeInt(0);
		snapshot->delta(tempLastSavedSnap);

		if (!snapshot->getPlayerstate()->isAtributeSet(0))
			snapshot->getPlayerstate()->setAtribute(0,commandTime);

		removeNotChanged(snapshot);
		snapshot->setDeltanum(message->getSeqNumber() - lastSavedSeqNumber);


		lastSavedSeqNumber = message->getSeqNumber();
		delete tempLastSavedSnap;
	} else if (snapshot) {
		lastSavedSnap = snapshot->clone();
		lastSavedSeqNumber = message->getSeqNumber();
	}

	message->saveMessage(ouputCopy);
	demoToOptimize.unloadMessage(messageId);
}



void processMessage(int messageId){
	Message*	message = demoToOptimize.getMessage(messageId);

	Snapshot* processedSnap = getFirstSnapshot(message);



	if (!processedSnap) { //this message has no snapshot, save it as it is

		//check for gamestate though, so we can remember that last message was gamestate one
		//and therefore not delta referencing upcoming snapshot
		for(int j=0; j < message->getInstructionsCount();++j){
			if (message->getInstruction(j)->getType() == INSTR_GAMESTATE){
				lastMessageWasGamestate = true;
				break;
			}
		}

		save(messageId);
		return;
	} 

	if (!lastSavedSnap || lastMessageWasGamestate){ //very first snapshot in demo
		removeNotChanged(processedSnap); //i think this should be deleted later
		save(messageId);
		lastMessageWasGamestate = false;
		return;
	}

	//here we have message with snapshot which is not first in demo
	Snapshot* processedSnapClone = processedSnap->clone();

	//we compress it according to last saved message
	processedSnapClone->delta(lastSavedSnap);
	//removeNotChanged(processedSnapClone);
	processedSnapClone->setDeltanum(message->getSeqNumber() - lastSavedSeqNumber);

	//does the snapshot differ?
	if (processedSnapClone->getPlayerstate()->getAtributesCount() == 0){
		//this does not differ from last saved message, keep in queue uncompressed
		//this message will be later interpolated
		delete processedSnapClone;

		//if there is not already something in queue, mark this as the start of queue
		if (!queueStartId)
			queueStartId = messageId;

		return;
	}

	//ok snapshot has changes from last saved message, save it
	delete processedSnapClone;

	if (queueStartId){
		int queueId;

		//we need to interpolate and save everything in queue
		for(queueId = queueStartId; queueId < messageId; ++queueId){
			Interpolate(getFirstSnapshot(demoToOptimize.getMessage(queueId)),lastSavedSnap,processedSnap);
			++interpolatedSoft;
		}

		//everything in queue is interpolatedSoft, lets save it one by one
		for(queueId = queueStartId; queueId < messageId; ++queueId){
			save(queueId);
		}

		queueStartId = 0;
	}

	//everything including queue is saved, save processing message finally
	save(messageId);

}

int main(int argc, char** argv){

	if (!CheckArguments(argc)){
		cout << "Wrong arguments format." << endl;
		cout << "Usage: Smoother [Input] {Output}" << endl;
		return 1;

	}

	string	demoName = argv[1];
	string  outputName;

	//process parameters
	for (int i=2;i < argc;++i){
		string para = argv[i];

		outputName = para;

	}

	if (outputName.empty()){
		outputName = demoName;

		int extPos = outputName.find_last_of('.');

		if (extPos == string::npos){
			demoName += ".dm_26";
			outputName += "_smooth.dm_26";
		} else {
			outputName.insert(extPos, "_smooth");
		}
	}

	ouputCopy.open(outputName,ios::binary);

	if (demoToOptimize.open(demoName.c_str())){
		cout << "Demo '" << demoName << "' successfully opened." << endl;
	}else{
		cout << "Failed to open demp '" << demoName << "'." << endl;
		return 1;
	}

	//analysis
	demoToOptimize.analyse();

	for(int i = 0; i < 1024; ++i){
		removedEntities[i] = false;
		notChanged[i] = false;
	}

	int newMessageId;
	Message 	    *newMessage;

//	Snapshot* lastSavedSnapshot = 0;
//	int	lastSavedSeqNumber = 0;

	clock_t init, final;

	int tick = 0;
	//cout << endl;

	init=clock();

	for(newMessageId = 0; newMessageId < demoToOptimize.getMessageCount(); ++newMessageId){
		newMessage = demoToOptimize.getMessage(newMessageId);
		clearServerCommands(newMessage); //removing redundant server commands
		uncompressMessage(newMessageId); //making it delta 0
		
		if (newMessageId >= 32){
			processMessage(newMessageId-32); //process message
		}

		if ((newMessageId*20)/(demoToOptimize.getMessageCount()-1) >= tick){
			int percent = (newMessageId*100)/(demoToOptimize.getMessageCount()-1);
			cout << "\b\b\b\b" << percent << "%";
			//cout << percent << "%" << endl;
			++tick;
		}
	}
	cout << endl;

	//process rest of messages
	for(; newMessageId < demoToOptimize.getMessageCount() + 32; ++newMessageId){
		if (newMessageId >= 32){
			processMessage(newMessageId-32); //process message
		}
	}

	//last messages can still be waiting in queue so we need to 
	//manually save all stuff in queue now, at least we dont need
	//to do interpolation (since we apparently havent reach next changing snapshot)
	if (queueStartId){
		int queueId;

		//everything in queue is interpolatedSoft, lets save it one by one
		for(queueId = queueStartId; queueId < demoToOptimize.getMessageCount(); ++queueId){
			save(queueId);
		}

		queueStartId = 0;
	}

	final=clock()-init;

    double s = (double)final / ((double)CLOCKS_PER_SEC);	

	//logfile.close();
	ouputCopy.close();
	cout << "Interpolated (SOFT) " << interpolatedSoft << " frames." << endl;
	cout << "Done in " << s << "s." << endl;	
	cout << "'" << outputName << "' saved." << endl;

	return 0;
}