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

#ifndef NODESTORE_H
#define NODESTORE_H

#include "../gameglobal.h"
#include "../global.h"
#include "../baseclass/basenode.h"
#include "../baseclass/traversal.h"

#include <QString>

typedef int storeId;
typedef vector<storeId> storeIdLink;

//Little class to store a position with special information useful when tracing
class NodeTrace {
  public :
	QString id;				//storeId of the position
	QString index;			//index indicating the child currently studied
	QString traversal_info;	//information about the travesal of the tree
	int alive;				//number of unknown children
	int bcu;				//number of unknown children of the best child
	QString nb_lives;		//number of lives of the position (for Sprouts only)
	QString nodePos;		//part of the node other than the position itself
	QString pos;			//position corresponding to this node
	QString posColor;		//background color for this position
	bool is_alive;			//false if the position has been computed W
};

//little class to store some data about a node
//(this is stored separately from the node to avoid programming errors - at least temporarily)
//Note : the number of children and other values is stored in short int inside of int to save memory
//       it implies that the current limitation on the number of children is 32767
class NodeData {
  public:
	NodeData();
	void initAgain();
	bool knownState;
	short int studied_pos_index;
	short int children_number;
	short int unknown_number;
	BaseTraversal traversal_data;
};

//class that contains all the elements of a given node
class Node {
  public:
	BaseNode storedBaseNode;
	NodeData storedNodeData;
	storeIdLink childrenIdList;
	storeIdLink parentIdList;
};

//class to store the Node used in the computation
class NodeStore {
	//------------------database to store the nodes------------
	public:
	NodeStore();
	static QMutex nodeStoreLock;
	static int debugLevel;
	static int mainloopCount;	//count the number of times the main loop was entered
	static int TRnodesCount;	//count the number of TRaversal nodes
	static int DFnodesCount;	//count the number of Depth-First nodes
	static int maxNodesCount;	//count the maximum number of all stored nodes 
	
	private:
	public: //needed temporarily for some traversal algo
	static storeId nextId;		//next available id for a new node
	static storeId rootId;		//id of the first node of the computation
	
	static map<string, storeId> idMap; //id corresponding to the string representation of a BaseNode
	static map<storeId, Node> nodeDataBase;
	
	static bool warningEnabled; //enable/disable warning messages
	
	static BaseTraversal traversalStatic; //object to call "static" functions of BaseTraversal
	
	//-------------------main functions-------------------------
	public: //these functions are thread-safe
	static void clearNodes(); 						//clear all the objects
	static storeId getRootId();
	static storeId addNode(/*const*/ BaseNode& a);
	static void updNode(const storeId &id, /*const*/ BaseNode& a);
	static storeIdLink addNodeList(/*const*/ list<BaseNode>& LNC);
	static void createLinks(storeId parentId, storeIdLink childrenIdList);
	static void deleteChildrenLink(const storeId &id);
	static void setNodeData(const storeId &id, NodeData d);
	static BaseNode externGetBaseNode(const storeId &id, bool &correctId);
	static NodeData externGetNodeData(const storeId &id, bool &correctId);
	static list<BaseNode> getChildrenBaseNode(const storeId &id, bool &correctId);
	static storeIdLink externGetChildrenList(const storeId &id, bool &correctId);
	static QString stateString(); //return a string with informations about the nodestore state
	
	private : //these functions are "internal" and must be called only from a function that is already thread-safe
	public : //needed temporarily for some traversal algo
	static void internalDeleteChildrenLink(const storeId &id);
	static void deleteParentLink(const storeId &parentId, const storeId &id);
	static void deleteNode(const storeId &id);
	static void deleteNodeString(const string s);
	
	static BaseNode getBaseNode(const storeId &id, bool &correctId);
	static NodeData getNodeData(const storeId &id, bool &correctId);
	static storeIdLink getChildrenList(const storeId &id, bool &correctId);
	static storeIdLink getParentList(const storeId &id, bool &correctId);
	static int childrenIndex(const storeId &parentId, const storeId &childrenId);
	static int computeBcu(const storeId &id);
	
	//-----------computing branch-------------------
	public:
	static void compBranchAdd(const storeId &id);	//add a new id at the end of the computing branch
	static void compBranchDel();						//delete the last id of the computing branch
	static void compBranchUpd(const storeId &id);	//update the last id of the computing branch
	static storeId compBranchLast();					//last id of the computing branch
	static bool compBranchIsEmpty();					//indicate if he computing branch is empty
	static int compBranchSize();
	static void createCompBranchTo(const storeId & targetId);
	
	private:
	public: //needed temporarily
	static list<storeId> computingBranch;				//list of ids of the computing branch
	static list<storeId> getComputingBranch();
	static storeId lastComputedId;					//last id computed before a deletion
	
	//-------main loop function--------------------
	public:
	static void mainLoop();							//main algorithm
	static storeId nextUnknownChild();			//next unknown child of the last id
	static storeId previousUnknownChild();		//previous unknown child of the last id
	static storeId userRequestId(int val);		//choose the child requested by the user
	
	//----------------trace------------------
	public:
	static list<NodeTrace> getMainTrace();
	static storeId getLevelStoreId(int level);
	static list<NodeTrace> getOneNodeTrace(const storeId &id);
	static list<NodeTrace> getChildrenTrace(const storeId &id);
	static list<NodeTrace> getParentsTrace(const storeId &id);
	static list<NodeTrace> traceFromList(const storeIdLink &list_id);
	
	//debug
	static string storeSize();						//return a string with all the database sizes
	static void checkConsistency(const storeId &id);
	
	//---------Transpositions------------
	//useful only with a traversal algorithm different from alpha-beta
	static bool useTranspositions;
	
	//---------Traversal-----------------
	static bool useTraversal;			//indicate if we use a traversal different from alpha-beta
	static storeId userChosenId; //different from -1 if the user has chosen a traversal id
};

#endif
