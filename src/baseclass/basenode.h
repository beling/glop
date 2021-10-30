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

#ifndef BASENODE_H
#define BASENODE_H

#include "../gameglobal.h"
#include "../global.h"
#include "interface.h"
#include "../moveordering.h"

class QString;
class QStringList;

//the following macro defines the list of function that should be reimplemented in a node
//it should be included in the public part of all Nodes
#define NODE_FUNCTION_LIST \
	virtual int startComputationKind(); \
	virtual GameType gameType(); \
	virtual void createDataStorage(); \
	virtual bool tryImmediateResult(); \
	virtual bool computeNodeChildren(list<BaseNode> &children); \
	virtual void resultFromAllChildren(list<BaseNode> &children); \
	virtual bool resultFromOneChild(BaseNode &child); \
	virtual bool resultIsComplete(); \
	virtual void computeOrderParameters(); \
	virtual Interface getParamDef(); \
	virtual void initRootNode(); \
	virtual string resultString(); \
	virtual QStringList infoAbout(string); \
	virtual QStringList displayStringList(); \
	virtual QString displayColor(); \
	virtual string debugStringKind(); \
	virtual string debugStringPos(); \
	virtual string debugStringResult();

//! @defgroup NodeGroup Nodes

//! Base class for defining a new kind of Node
//! @ingroup NodeGroup
class BaseNode {
  private:
	BaseNode * p_baseNode;
	static vector<BaseNode *>& nodeList();
	void initializePointer(int nodeIndex);
	
  public:
	//register mecanism
	static int registerNewNode(BaseNode *a);  //!< add a new node to the list of available nodes
	static int nodesNumber();
	bool validPointer() const;
	BaseNode * getPointer() const;
	BaseNode();
	BaseNode(int nodeIndex, int compIni);
	BaseNode(const BaseNode& a);
	BaseNode& operator=(const BaseNode& a);
	virtual ~BaseNode();
	virtual BaseNode* clone() const;
	
	bool operator==(const BaseNode& a) const;
	bool operator<(const BaseNode& a) const;
	
	NODE_FUNCTION_LIST
	
	//optional function
	virtual unsigned int traversalHeuristic();	//heuristic value for some traversal algorithms
	virtual void prepareNextCycle();			//used to prepare the next cycle (transfer databases) during random check cycles
	virtual bool lockedComputation();			//indicate if the user is allowed to interact with this kind of node
	virtual string getPositionName();  			//name of the start position set by the user in the interface
	virtual string extraPositionPartTitle();	//title for the column with the part of the node that is not the position
	
	//actions to be done when two positions collide during file processing
	virtual bool treatBaseCollision(const string &pos, nimber nBase, nimber nFile, int baseIndex, bool isCheckBase);
	virtual bool treatBaseCollision(const string &pos, string sBase, string sFile, int baseIndex, bool isCheckBase);
	
	static string computationResult;
	static string startPositionString;
	
	//-------------------------------------------------------------------------------------------
	int computation_kind; 		//integer id to distinguish between the different kind of algorithm
	MoveOrdering nodeOrder;		//object used by sortChildren to define the priorities
	
	void setComputationKind(int compIni);
	int getComputationKind();
	MoveOrdering getMoveOrdering();
	void sortChildren(list<BaseNode> &children, bool unique);
	void clearMoveOrdering();
	string nodeStoreString();		//used by nodestore
};

//the following macro must be included in the public section of any class derived from BaseNode
//It provides the clone, access and register mecanism for the node
#define NODE_REGISTER_MACRO(NODE_NAME) \
	NODE_NAME() {}; \
	virtual BaseNode* clone() const { return new NODE_NAME(*this); }; \
	static NODE_NAME * access(BaseNode &nc) { return (NODE_NAME *) nc.getPointer(); }; \
	NODE_NAME(int) { registeredIndex() = BaseNode::registerNewNode(this); }; \
	static NODE_NAME root; \
	static int& registeredIndex() { static int index; return index; };
	
#endif
