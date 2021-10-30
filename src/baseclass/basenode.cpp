/***************************************************************************
 *   Copyright (C) 2006-2011 by Julien Lemoine, Simon Viennot              *
 *   lapinot@tuxfamily.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "basenode.h"
#include "basegame.h"

#include <QStringList>

string BaseNode::computationResult;
string BaseNode::startPositionString;

vector<BaseNode *>& BaseNode::nodeList() {
	static vector<BaseNode *> internalList;
	return internalList;
}

//add a new node to the node list
//return the identifier for this node
int BaseNode::registerNewNode(BaseNode *a) {
	nodeList().push_back(a);
	
	//return the index in the list
	return (nodeList().size() - 1);
}

int BaseNode::nodesNumber() {
	return (int) (nodeList().size());
}

void BaseNode::initializePointer(int nodeIndex) {
	//initialize the internal pointer with the correct node object
	if(nodeIndex<(int)nodeList().size()) { //security
		//create a new object of the correct node by cloning
		p_baseNode = (nodeList()[nodeIndex])->clone();
	} else {
		cout << "BaseNode error : unknown node" << endl;
		p_baseNode = 0;
	}
}

BaseNode::BaseNode() {
	p_baseNode=0;
}

BaseNode::BaseNode(int nodeIndex, int compIni) {
	initializePointer(nodeIndex);
	setComputationKind(compIni);
}

BaseNode::~BaseNode() {
	if(p_baseNode!=0) {
		delete p_baseNode;
		p_baseNode=0;
	}
}

//we need to copy the object pointed by p_baseNode and not the pointer itself
BaseNode::BaseNode(const BaseNode& a) {
	computation_kind = a.computation_kind;
	nodeOrder = a.nodeOrder;
	
	if(a.p_baseNode != 0) {
		p_baseNode = a.p_baseNode->clone();
	} else {
		p_baseNode = 0;
	}
}

BaseNode& BaseNode::operator=(const BaseNode& a) {
	//protection against memory leak in case of code like n = n
	if(this != &a) {
		computation_kind = a.computation_kind;
		nodeOrder = a.nodeOrder;
		
		//delete the old p_baseNode
		if(p_baseNode!=0) {
			delete p_baseNode;
			p_baseNode=0;
		}
		
		if(a.p_baseNode != 0) {
			p_baseNode = a.p_baseNode->clone();
		} else {
			p_baseNode = 0;
		}
	}
	
	return *this;
}

//note:this function is never called (always reimplemented in the derived class)
BaseNode* BaseNode::clone() const {
	return new BaseNode(*this);
}

//return true if the pointer is valid (not null)
bool BaseNode::validPointer() const {
	if(p_baseNode==0) {
		cout << "BaseNode::p_baseNode is Null. Segmentation Fault is going to occur." << endl;
		return false;
	} else {
		return true;
	}
}

BaseNode * BaseNode::getPointer() const {
	validPointer();
	return p_baseNode;
}

//compare operator for nodes
bool BaseNode::operator<(const BaseNode& a) const{
	return ( (p_baseNode->nodeOrder) < (a.p_baseNode->nodeOrder) );
}

bool BaseNode::operator==(const BaseNode& a) const {
	return( p_baseNode->debugStringPos() == a.p_baseNode->debugStringPos() );
}

//----------call to baseNode functions -------------

void BaseNode::setComputationKind(int compIni) {
	if(p_baseNode!=0) {
		p_baseNode->setComputationKind(compIni);
	} else {
		computation_kind = compIni;
	}
}

int BaseNode::getComputationKind() {
	if(p_baseNode!=0) {
		return p_baseNode->getComputationKind();
	} else {
		return computation_kind;
	}
}

MoveOrdering BaseNode::getMoveOrdering() {
	if(p_baseNode!=0) {
		return p_baseNode->getMoveOrdering();
	} else {
		return nodeOrder;
	}
}

int BaseNode::startComputationKind() {
	validPointer();
	return p_baseNode->startComputationKind();
}

GameType BaseNode::gameType() {
	validPointer();
	return p_baseNode->gameType();
}

bool BaseNode::resultIsComplete() {
	validPointer();
	return p_baseNode->resultIsComplete();
}

bool BaseNode::tryImmediateResult() {
	validPointer();
	return p_baseNode->tryImmediateResult();
}

bool BaseNode::computeNodeChildren(list<BaseNode> &children) {
	validPointer();
	return p_baseNode->computeNodeChildren(children);
}

void BaseNode::resultFromAllChildren(list<BaseNode> &children) {
	validPointer();
	p_baseNode->resultFromAllChildren(children);
}

bool BaseNode::resultFromOneChild(BaseNode &child) {
	validPointer();
	return p_baseNode->resultFromOneChild(child);
}

void BaseNode::computeOrderParameters() {
	validPointer();
	p_baseNode->computeOrderParameters();
}

Interface BaseNode::getParamDef() {
	validPointer();
	Interface interface = p_baseNode->getParamDef();
	
	//we add to all interfaces a hidden variable used to store the name of the start position
	interface.addHiddenString("");
	interface.link(startPositionString, "initPosition");
	
	//we add to all interfaces a hidden variable used to store the result of a computation
	interface.addHiddenString("");
	interface.link(computationResult, "Result");
	return interface;
}

void BaseNode::initRootNode() {
	validPointer();
	p_baseNode->initRootNode();
}

void BaseNode::createDataStorage() {
	validPointer();
	p_baseNode->createDataStorage();
}

string BaseNode::resultString() {
	validPointer();
	return p_baseNode->resultString();
}

QString BaseNode::displayColor() {
	validPointer();
	return p_baseNode->displayColor();
}

QStringList BaseNode::displayStringList() {
	validPointer();
	return p_baseNode->displayStringList();
}

//title for the column with the part of the node that is not the position
//by default, title is an empty string, which means that the column is not displayed
string BaseNode::extraPositionPartTitle() {
	if(p_baseNode!=0) {
		return p_baseNode->extraPositionPartTitle();
	} else {
		return string("");
	}
}

string BaseNode::debugStringKind() {
	validPointer();
	return p_baseNode->debugStringKind();
}

string BaseNode::debugStringPos() {
	validPointer();
	return p_baseNode->debugStringPos();
}

string BaseNode::debugStringResult() {
	validPointer();
	return p_baseNode->debugStringResult();
}

//nodestore string
//caution : we need a string really unique for a given node,
// that is a string with ALL the node information
string BaseNode::nodeStoreString() {
	if(p_baseNode!=0) {
		return p_baseNode->nodeStoreString();
	} else {
		//the string is a combination of computation_kind and displayStringList
		string result;
		result += QString("%1=").arg(computation_kind).toStdString();
		QStringList displayString = displayStringList();
		result += displayString.at(1).toStdString();
		if(displayString.size()>1) result += string("=") + displayString.at(2).toStdString();
		return result;
	}
}

QStringList BaseNode::infoAbout(string g) {
	validPointer();
	return p_baseNode->infoAbout(g);
}

//compute the parameters of each child of the list
//unique indicate if we should remove duplicate children
void BaseNode::sortChildren(list<BaseNode> &children, bool unique) {
	//compute the sort parameters
	list<BaseNode>::iterator Ni;
	for(Ni=children.begin(); Ni!=children.end(); Ni++) {
		Ni->computeOrderParameters();
	}
	
	if(!unique) {
		//simply sort the children and return
		children.sort();
		
		//free some of the memory used by the parameters
		for(Ni=children.begin(); Ni!=children.end(); Ni++) {
			Ni->clearMoveOrdering();
		}
		return;
	}
	
	//------remove of the duplicates is needed----------
	if(Parameter::isCheck && (Parameter::RandomCheckCycles>1 || (Parameter::RandomCheckCycles==1 && !Parameter::LastCycleIsNormal) ) ) {
		//special case of random computations
		//we need first to sort children without taking checkValue into account, in order to remove duplicates
		MoveOrdering::ignoreCheckValue = true;
		children.sort();
		children.unique();	//remove duplicates
		
		//we need a second sort to take into account the random checkValue
		MoveOrdering::ignoreCheckValue = false;
		children.sort();
	} else {
		children.sort();	//sort children
		children.unique();	//remove duplicates
	}
	
	//free some of the memory used by the parameters
	for(Ni=children.begin(); Ni!=children.end(); Ni++) {
		Ni->clearMoveOrdering();
	}	
}

//clear the moveOrdering object (to save some memory)
void BaseNode::clearMoveOrdering() {
	if(p_baseNode!=0) {
		//we need to clear the internal object
		p_baseNode->clearMoveOrdering();
	} else {
		//this is the real object
		nodeOrder.clear();
	}
}

//return a heuristic value for the traversal initialization
//if the function is not implemented in a node, the default value is 1
unsigned int BaseNode::traversalHeuristic() {
	if(p_baseNode!=0) {
		return p_baseNode->traversalHeuristic();
	} else {
		return 10;		//default value
	}
}


//return the name of the starting position, but also stores it in startPostionString
//it will be saved in xml files
string BaseNode::getPositionName() {
	if(p_baseNode!=0) {
		startPositionString = p_baseNode->getPositionName();
		return startPositionString;
	} else {
		//the default implementation simply uses the position name of the game
		return BaseGame(Parameter::gameChoice).getPositionName();
	}
}

//this function is optional
void BaseNode::prepareNextCycle() {
	validPointer();
	return p_baseNode->prepareNextCycle();
}

//this function is optional
//indicate if we can interact with this kind of node
//by default, check computations are locked
bool BaseNode::lockedComputation() {
	if(p_baseNode!=0) {
		return p_baseNode->lockedComputation();
	} else {
		//default value
		if(Parameter::isCheck) {
			return true;
		} else {
			return false;
		}
	}
}

//actions to be done when two positions collide during file processing
//return true if the collision could be treated correctly
//Case of a (string, nimber) database
bool BaseNode::treatBaseCollision(const string & pos, nimber nBase, nimber nFile, int baseIndex, bool isCheckBase) {
	if(p_baseNode!=0) {
		return p_baseNode->treatBaseCollision(pos, nBase, nFile, baseIndex, isCheckBase);
	} else {
		//default action, we only check that the values are the same
		return (nBase == nFile);
	}
}

//actions to be done when two positions collide during file processing
//return true if the collision could be treated correctly
//Case of a (string, string) database
bool BaseNode::treatBaseCollision(const string & pos, string sBase, string sFile, int baseIndex, bool isCheckBase) {
	if(p_baseNode!=0) {
		return p_baseNode->treatBaseCollision(pos, sBase, sFile, baseIndex, isCheckBase);
	} else {
		//default action, we only check that the values are the same
		return (sBase == sFile);
	}
}
