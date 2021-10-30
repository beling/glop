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

#include "nodestore.h"
#include "trace.h"

#include <QMutex>

void printList(list<storeId> Lid) {
	list<storeId>::const_iterator Si;
	for(Si=Lid.begin(); Si!=Lid.end(); Si++) {
		cout << "#" << *Si << " ";
	}
	cout << endl;
}

void printList(vector<storeId> Lid) {
	vector<storeId>::const_iterator Si;
	for(Si=Lid.begin(); Si!=Lid.end(); Si++) {
		cout << "#" << *Si << " ";
	}
	cout << endl;
}

NodeData::NodeData() {
	knownState=false;
	studied_pos_index=-1;
	children_number=0;
	unknown_number=0;
	
	//init traversal_data with the correct traversal object
	if(Parameter::isCheck) {
		//in case of a check computation, always use an alpha-beta algorithm (= no traversal)
		BaseTraversal bt(BaseTraversal::noTraversalIndex());
		traversal_data = bt;
	} else {
		//in case of a normal computation, use the choice of the user
		BaseTraversal bt(Parameter::traversalChoice);
		traversal_data = bt;
	}
	traversal_data.init();
}

//called after one step in a multi-step algorithm
void NodeData::initAgain() {
	knownState=false;
	studied_pos_index=-1;
	children_number=0;
	unknown_number=0;
	traversal_data.initAgain();
}

//-----------static objects of NodeStore ------------------
QMutex NodeStore::nodeStoreLock;
int NodeStore::debugLevel=0;
int NodeStore::mainloopCount=0;
int NodeStore::TRnodesCount=0;  //count the number of TRaversal nodes
int NodeStore::DFnodesCount=0;  //count the number of Depth-First nodes
int NodeStore::maxNodesCount=0;  //count the maximum number of stored nodes

storeId NodeStore::nextId=0;
storeId NodeStore::rootId=0;
map<string, storeId> NodeStore::idMap; //id corresponding to the string representation of a BaseNode
map<storeId, Node> NodeStore::nodeDataBase;

list<storeId> NodeStore::computingBranch;
storeId NodeStore::lastComputedId = -1;

bool NodeStore::useTranspositions=false;

//static variable indicating if we are using a tree traversal algorithm (different from alpha-beta)
bool NodeStore::useTraversal=false; 
storeId NodeStore::userChosenId=-1;

//enable/disable warning messages
//in most cases, warning messages are better
//a notable execption is the children trace because we can try to accept to nodes that don't exist
bool NodeStore::warningEnabled=true; 

BaseTraversal NodeStore::traversalStatic; //object to call "static" functions of BaseTraversal

//--------------implementation of NodeStore ----------------

NodeStore::NodeStore() {
	nextId = 0;
	lastComputedId = 1;
}

storeId NodeStore::getRootId() {
	QMutexLocker locker(&nodeStoreLock);
	return rootId;
}

void NodeStore::clearNodes() {
	QMutexLocker locker(&nodeStoreLock);
	idMap.clear();
	nodeDataBase.clear();
	nextId = 0;
	DFnodesCount=0;
	TRnodesCount=0;
	computingBranch.clear();
	lastComputedId = 1;
}

storeId NodeStore::addNode(/*const*/ BaseNode& a) {
	QMutexLocker locker(&nodeStoreLock);
	if(!a.validPointer()) {
		cout << "NodeStore::addNode : Invalid pointer of a BaseNode in NodeStore::addNode" << endl;
	}
	
	//transpositions : don't create a new node, if the same node already exists
	if(NodeStore::useTranspositions) {
		string nodeString = a.nodeStoreString();
		map<string, storeId>::iterator it = idMap.find(nodeString);
		if(it!=idMap.end()) {
			if(debugLevel==4) {
				cout << "(addNode) Exist id #" << it->second << "  " << nodeString << endl;
			}
			return it->second;
		}
	}
	
	//add the node with a new storeId
	storeId result = nextId;
	//idMap[nodeString]=nextId;
	
	Node newNode;
	newNode.storedBaseNode=a;
	if(useTraversal) newNode.storedNodeData.traversal_data.initFromNode(a);		//Warning : initFromNode is authorized to modify the object a
	nodeDataBase[nextId]=newNode;
	
	if(debugLevel==4) {
		string nodeString = a.nodeStoreString();
		cout << "(addNode) New id #" << nextId << "  " << nodeString << endl;
	}
	nextId++;
	if(nextId<0) cout << "Too long computation. Node Id overflow in NodeStore." << endl;
	
	if(newNode.storedNodeData.traversal_data.isTraversalNode()) {
		TRnodesCount++;
	} else {
		DFnodesCount++;
	}
	if((int) nodeDataBase.size() > maxNodesCount) maxNodesCount = nodeDataBase.size();
	
	return result;
}

//the lock is done in addNode
storeIdLink NodeStore::addNodeList(/*const*/ list<BaseNode>& LNC) {
	storeIdLink result;
	result.reserve(LNC.size());	//storeIdLink is a vector, it is more efficient to reserve the needed memory
	
	list<BaseNode>::iterator NCi;
	for(NCi=LNC.begin(); NCi!=LNC.end(); NCi++) {
		result.push_back(addNode(*NCi));
	}
	return result;
}

void NodeStore::updNode(const storeId &id, /*const*/ BaseNode& a) {
	QMutexLocker locker(&nodeStoreLock);
	if(!a.validPointer()) {
		cout << "Invalid pointer of BaseNode in NodeStore::updNode #" << id << " " << endl;;
	}

	//first step : find the current node for this id
	map<storeId, Node>::iterator it = nodeDataBase.find(id);
	if(it!=nodeDataBase.end()) {
		if(NodeStore::useTranspositions) {
			string s = it->second.storedBaseNode.nodeStoreString();	//get the string representation of the old node
			deleteNodeString(s);									//delete the old string from the index
			idMap[a.nodeStoreString()]=id;							//update the string index
		}
		
		//update the nodeDataBase
		it->second.storedBaseNode = a;
	} else {
		cout << "NodeStore::updNode error : id doesn't exist" << endl;
		return;
	}
}

//add links parent->children and children->parent
void NodeStore::createLinks(storeId parentId, storeIdLink childrenIdList) {
	QMutexLocker locker(&nodeStoreLock);
	map< storeId, Node >::iterator it = nodeDataBase.find(parentId);
	if(it!=nodeDataBase.end()) {
		//initialize the list of children
		it->second.childrenIdList = childrenIdList;
		
		if(debugLevel==4) {
			cout << "(createLinks) parent #" << parentId << " children ";
			printList(childrenIdList);
		}
	} else {
		cout << "NodeStore::createLinks error : id doesn't exist" << endl;
		return;
	}
	
	//look for the parent of each child to add parentId
	storeIdLink::const_iterator childId;
	for(childId=childrenIdList.begin(); childId!=childrenIdList.end(); childId++) {
		it = nodeDataBase.find(*childId);
		if(it!=nodeDataBase.end()) {
			it->second.parentIdList.push_back(parentId);
		} else {
			cout << "NodeStore::createLinks error : childId doesn't exist" << endl;
			return;			
		}
	}
}

//delete the children link of the node
//indicate in the node data that there are no more children
void NodeStore::deleteChildrenLink(const storeId &id) {
	QMutexLocker locker(&nodeStoreLock);
	
	//------------indicate that the node has no more children------
	bool correctId; //not used
	NodeData data = getNodeData(id, correctId);  //nodeData
	data.children_number=0;
	data.studied_pos_index=-1;
	nodeDataBase[id].storedNodeData = data;
	
	//---------delete the links to the children (recursive)--------
	internalDeleteChildrenLink(id);
}

//internal function (no lock)
void NodeStore::internalDeleteChildrenLink(const storeId &id) {
	if(id<0 || id>nextId) return; //id is out of the posible range
	//cout << "D children of " << id << endl;
	
	//find children and recursively erase them
	map< storeId, Node >::iterator it2 = nodeDataBase.find(id);
	if(it2==nodeDataBase.end()) {
		cout << "NodeStore::internalDeleteChildrenLink : Trying to delete the children of an incorrect id #" << id << " " << endl;
		return;
	}
	
	if(it2->second.childrenIdList.empty()) return; //the node has no child
	
	storeIdLink::const_iterator Si;
	for(Si=it2->second.childrenIdList.begin(); Si!=it2->second.childrenIdList.end(); Si++) {
		deleteParentLink(id, *Si);
	}
	
	//since storeIdLink is a vector, the best way to clear and free the memory is a swap with empty vector
	//it2->second.childrenIdList.clear();
	storeIdLink VoidLink;
	it2->second.childrenIdList.swap(VoidLink);
}

//internal function (no lock)
void NodeStore::deleteParentLink(const storeId &parentId, const storeId &id) {
	if(debugLevel==4) {
		cout << "(DeleteParentLink) Parent : #" << parentId << " Child : #" << id << " " << endl;
	}
	
	//erase the parent from the list of parents
	map<storeId, Node>::iterator it = nodeDataBase.find(id);
	if(it!=nodeDataBase.end()) {
		
		//TODO for algorithm with transpositions : remove only the parentID
		//here, we remove all the parents
		//it->second.parentIdList.remove(parentId);
		it->second.parentIdList.clear();
		
		//if there is no more parent for the node, delete the node
		if(it->second.parentIdList.empty()) {
			//delete the node
			//note : after this call, the iterator id is not valid anymore
			deleteNode(id);
		}
	} else {
		cout << "NodeStore::deleteParentLink : Trying to delete the parentid of an incorrect id #" << id << " " << endl;
		return;
	}
}

//internal function (no lock)
//a node delete recursively all its children
void NodeStore::deleteNode(const storeId &id) {
	if(id<0 || id>nextId) return; //id is out of the posible range
	if(debugLevel==4) {
		cout << "(DeleteNode) #" << id << " " << endl;
	}
	
	internalDeleteChildrenLink(id);
	
	//cout << "D id : " << id << endl;
	map<storeId, Node>::iterator it = nodeDataBase.find(id);
	if(it!=nodeDataBase.end()) {
		if(NodeStore::useTranspositions) {
			//erase the string representation of the node from the index
			deleteNodeString(it->second.storedBaseNode.nodeStoreString());
		}
		
		//we are deleting a node : update the DF or traversal number of nodes
		if(useTraversal && it->second.storedNodeData.traversal_data.isTraversalNode()) {
			TRnodesCount--;
		} else {
			DFnodesCount--;
		}
		
		//erase the node from nodeDataBase
		nodeDataBase.erase(it);
	}
}

//internal function (no lock)
//delete a string from the <string, id> map
void NodeStore::deleteNodeString(const string s) {
	map<string, storeId>::iterator it = idMap.find(s);
	if(it!=idMap.end()) {
		idMap.erase(it);
	}
}

//external function : call the internal function with a lock
BaseNode NodeStore::externGetBaseNode(const storeId &id, bool &correctId) {
	QMutexLocker locker(&nodeStoreLock);
	return getBaseNode(id, correctId);
}

//external function : call the internal function with a lock
NodeData NodeStore::externGetNodeData(const storeId &id, bool &correctId) {
	QMutexLocker locker(&nodeStoreLock);
	return getNodeData(id, correctId);
}

//internal function (no lock)
BaseNode NodeStore::getBaseNode(const storeId &id, bool &correctId) {
	map<storeId, Node>::iterator it = nodeDataBase.find(id);
	if(it!=nodeDataBase.end()) {
		correctId=true;
		if(it->second.storedBaseNode.validPointer()!=true) {
			cout << "NodeStore::getBaseNode : Invalid pointer of a BaseNode in getBaseNode id #" << id << " " << endl;
			correctId = false;
		}
		return it->second.storedBaseNode;
	} else {
		correctId=false;
		if(warningEnabled) {
			cout << "NodeStore::getBaseNode : Trying to get the BaseNode of an incorrect id #" << id << " " << endl;
		}
		BaseNode a;
		return a;
	}
}

//internal function (no lock)
//get the nodeData corresponding to a given id
NodeData NodeStore::getNodeData(const storeId &id, bool &correctId) {
	map<storeId, Node>::iterator it = nodeDataBase.find(id);
	if(it!=nodeDataBase.end()) {
		correctId=true;
		return it->second.storedNodeData;
	} else {
		correctId=false;
		if(warningEnabled) {
			cout << "NodeStore::getNodeData : Trying to get the NodeData of an incorrect id #" << id << " " << endl;
		}
		NodeData a;
		return a;
	}
}

//set or update the nodeData of a given id
void NodeStore::setNodeData(const storeId &id, NodeData d) {
	QMutexLocker locker(&nodeStoreLock);
	if(id<0) return; //usually -1, a node with an uninitialized value
	
	if(debugLevel==4) {
		cout << "(setNodeData) Id #" << id << " value " << d.studied_pos_index << " / " << d.children_number << endl;
	}
	nodeDataBase[id].storedNodeData = d;
}

//external function : call the internal function with a lock
storeIdLink NodeStore::externGetChildrenList(const storeId &id, bool &correctId) {
	QMutexLocker locker(&nodeStoreLock);
	return getChildrenList(id, correctId);
}

//internal function (no lock)
storeIdLink NodeStore::getChildrenList(const storeId &id, bool &correctId) {
	map< storeId, Node >::iterator it = nodeDataBase.find(id);
	if(it!=nodeDataBase.end()) {
		correctId=true;
		return it->second.childrenIdList;
	} else {
		correctId=false;
		if(warningEnabled) {
			cout << "NodeStore::getChildrenList : Trying to get" << 
			        "the children list of an incorrect id #" << id << " " << endl;
		}
		storeIdLink a;
		return a;
	}
}

//internal function (no lock)
storeIdLink NodeStore::getParentList(const storeId &id, bool &correctId) {
	map< storeId, Node >::iterator it = nodeDataBase.find(id);
	if(it!=nodeDataBase.end()) {
		correctId=true;
		return it->second.parentIdList;
	} else {
		correctId=false;
		if(warningEnabled) {
			cout << "NodeStore::getParentList : Trying to get" <<
			        " the parent list of an incorrect id #" << id << " " << endl;
		}
		storeIdLink a;
		return a;
	}
}

list<BaseNode> NodeStore::getChildrenBaseNode(const storeId &id, bool &correctId) {
	QMutexLocker locker(&nodeStoreLock);
	list<BaseNode> result;
	map< storeId, Node >::iterator it = nodeDataBase.find(id);
	if(it!=nodeDataBase.end()) {
		correctId=true;
		storeIdLink::iterator it2;
		for(it2=it->second.childrenIdList.begin() ; it2!=it->second.childrenIdList.end() ; it2++) {
			BaseNode c = getBaseNode(*it2, correctId);
			result.push_back(c);
		}
	} else {
		correctId=false;
		cout << "NodeStore::getChildrenBaseNode : Trying to get the children BaseNode list of an incorrect id #" << id << " " << endl;
	}
	return result;
}

//internal function (no lock)
//return the index of the given children in the children list of the parent
int NodeStore::childrenIndex(const storeId &parentId, const storeId &childrenId) {
	bool correctId; //unused
	storeIdLink childrenIdList = getChildrenList(parentId , correctId);
	storeIdLink::iterator it;
	int i;
	for(it=childrenIdList.begin(), i=0; it!=childrenIdList.end(); it++, i++) {
		if(*it == childrenId) {
			return i;
		}
	}
	
	//error case
	cout << "NodeStore::childrenIndex error : #" << childrenId << " is not a child of #" << parentId << endl;
	return -1;
}

//internal function (no lock)
//compute the bcu of a given node (-1 if the value is not known)
//bcu = "best child unknown" = the number of children of the child with the least number of children
int NodeStore::computeBcu(const storeId &id) {
	bool correctId;
	storeIdLink childrenIdList = getChildrenList(id , correctId);
	storeIdLink::iterator it;
	int bcu=-1;
	for(it=childrenIdList.begin(); it!=childrenIdList.end(); it++) {
		NodeData a = getNodeData(*it, correctId);
		if(correctId) {
			//don't take into account already killed children
			if(a.unknown_number>0 && !a.knownState) {
				//update the bcu if it has not been updated yet or if the new unknown number is smaller
				if(bcu==-1 || a.unknown_number<bcu) bcu = a.unknown_number;
			}
		}
	}
	return bcu;
}

//------------NodeStore computing branch---------------------

//add a new id at the end of the computing branch
void NodeStore::compBranchAdd(const storeId &id) {
	Trace::create_level();
	
	//uncomment this line to check consistency of all nodes before they are added to the computing branch
	//checkConsistency(id);
	
	QMutexLocker locker(&nodeStoreLock);
	
	//if it is not the first id, update the studied index of the parent
	if(!computingBranch.empty()) {
		storeId parentId = computingBranch.back();
		bool correctId;
		NodeData parentData = getNodeData(parentId, correctId);
		parentData.studied_pos_index = childrenIndex(parentId, id);
		nodeDataBase[parentId].storedNodeData = parentData;
	}
	
	computingBranch.push_back(id);
	lastComputedId = -1;
	if(debugLevel==4) {
		cout << "(compBranchAdd) ";
		printList(computingBranch);
	}
}

//delete the last id of the computing branch
void NodeStore::compBranchDel() {
	Trace::delete_level();
	
	QMutexLocker locker(&nodeStoreLock);
	if(!computingBranch.empty()) {
		lastComputedId = computingBranch.back();
		computingBranch.pop_back();
	}
	if(debugLevel==4) {
		cout << "(compBranchDel) ";
		printList(computingBranch);
	}
}

//update the last id of the computing branch
void NodeStore::compBranchUpd(const storeId &id) {
	QMutexLocker locker(&nodeStoreLock);
	list<storeId>::reverse_iterator it = computingBranch.rbegin();
	if(it!=computingBranch.rend()) {
		*it=id;
		lastComputedId = -1;
	} else {
		cout << "Trying to update an empty NodeStore::computingBranch" << endl;
	}
	if(debugLevel==4) {
		cout << "(compBranchUpd) ";
		printList(computingBranch);
	}
}

storeId NodeStore::compBranchLast() {
	QMutexLocker locker(&nodeStoreLock);
	if(!computingBranch.empty()) {
		return computingBranch.back();
	} else {
		cout << "Trying to get the last id of an empty computing branch in NodeStore::compBranchLast" << endl;
		return 0;
	}
}

bool NodeStore::compBranchIsEmpty() {
	QMutexLocker locker(&nodeStoreLock);
	return computingBranch.empty();
}

int NodeStore::compBranchSize() {
	QMutexLocker locker(&nodeStoreLock);
	int s = computingBranch.size();
	return s;
}

//internal function (no lock)
list<storeId> NodeStore::getComputingBranch() {
	return computingBranch;
}

//init the computing branch with the correct ids to reach the requested chosen id
void NodeStore::createCompBranchTo(const storeId & targetId) {
	if(debugLevel==4) cout << "createCompBranchTo : requested Id #" << targetId << endl;
	
	//find the correct list of nodes that go from the root to the requested id
	nodeStoreLock.lock();
	storeIdLink parentList;
	list<storeId> newComputingBranch;
	bool correctId;
	
	//check that the requested node is not already computed
	//correctId is set to false if the node is already computed (and we will do nothing)
	NodeData requestedNodeData = getNodeData(targetId, correctId);
	if(requestedNodeData.knownState) correctId = false;
	
	storeId previousId=targetId; // start from the target id and go back to the root
	while(correctId && previousId!=rootId) {
		newComputingBranch.push_front(previousId);
		parentList = getParentList(previousId, correctId);
		if(correctId && !parentList.empty()) {
			previousId = parentList.front(); //any parent is ok, so choose the first one
		} else {
			correctId = false;
		}
	}
	newComputingBranch.push_front(rootId);
	nodeStoreLock.unlock();
	
	//copy the new branch in the normal branch if we found a path from the root
	//we do nothing if we have not found a correct path
	if(correctId) {
		//---------find the size of the common part between the new computing branch and the old one
		list<storeId>::iterator Ni, Mi;
		bool stillEqual = true;
		int common = 0;
		for(Ni=newComputingBranch.begin(), Mi=computingBranch.begin(); 
		    Ni!=newComputingBranch.end() && Mi!=computingBranch.end() && stillEqual;
			Ni++, Mi++) {
			if(*Ni == *Mi) {
				common++;
			} else {
				stillEqual = false;
			}
		}
		
		//---------delete the non-common part of the computing branch
		int deleteSize = compBranchSize() - common;
		int i;
		for(i=0; i<deleteSize; i++) {
			compBranchDel();
			deleteChildrenLink(lastComputedId);  //delete the children
		}
		
		//------add the last part of the new computing branch
		for(Ni=newComputingBranch.begin(); Ni!=newComputingBranch.end(); Ni++, common--) {
			if(common <= 0) {
				//we have reached the non-common part
				//check that the node is still valid before adding it (to avoid any crash)
				correctId=true;
				NodeData data = externGetNodeData(*Ni, correctId);
				if(!correctId) {
					cout << "NodeStore::createCompBranchTo : aborted. The node " << *Ni << " is not valid" << endl;
					return;
				}
				
				//add the node to the computing branch
				compBranchAdd(*Ni);
			}
		}
	}
}

//--------------------------------Trace ---------------------------------------
//-----------------------------------------------------------------------------

//return a string with global informations about the nodestore state
QString NodeStore::stateString() {
	QMutexLocker locker(&nodeStoreLock);
	QString result("    Search-tree :\n");
	if(useTraversal) {
		result += QString("    TR nodes   : %1\n").arg(TRnodesCount);
	} else {
		result += QString("\n");
	}
	result += QString("    DF nodes   : %1\n").arg(DFnodesCount);
	result += QString("    Max nodes  : %1\n").arg(maxNodesCount);
	result += QString("    Iterations : %1").arg(mainloopCount);
	return result;
}

//get all the information needed for the main trace with only one function
list<NodeTrace> NodeStore::getMainTrace() {
	QMutexLocker locker(&nodeStoreLock);
	list<NodeTrace> completeResult;
	
	list<storeId> listS = NodeStore::getComputingBranch();
	list<storeId>::iterator it, previousIt;
	
	//main loop on the list of ids in the computing Branch
	for(it=listS.begin(); it!=listS.end(); it++) {
		NodeTrace result;
		result.posColor = QString("transparent");
		result.traversal_info = QString("");
		result.is_alive = true;
		
		previousIt=it;
		if(previousIt!=listS.begin()) {
			previousIt--;
		}
		
		bool correctId;
		NodeData d = NodeStore::getNodeData(*previousIt, correctId);
		if(!correctId) return completeResult;
		
		NodeData d_current = NodeStore::getNodeData(*it, correctId);
		if(!correctId) return completeResult;
		
		BaseNode c = NodeStore::getBaseNode(*it, correctId);
		if(!correctId) return completeResult;
		
		//index
		if(it!=listS.begin()) {
			result.index  = QString("%1 ").arg( d.studied_pos_index + 1, 2, 10 );
			result.index += QString("/");
			result.index += QString("%1 ").arg(d.children_number, 2, 10 );
			result.alive =d.unknown_number; 
		} else {
			//particular case of the first node
			result.index = QString("%1 ").arg( 1, 2, 10 ) + QString("/") + QString("%1 ").arg( 1, 2, 10 );
			result.alive = 1;
		}
		
		//Tree Traversal
		//print only the data of traversal-type nodes, and the data of the root node
		if(useTraversal && (d_current.traversal_data.isTraversalNode() || it==listS.begin())) {
			result.traversal_info = d_current.traversal_data.getMainTrace();
		}
		
		//number of lives, nimber, position
		QStringList disp = c.displayStringList();
		result.nb_lives = disp.at(0);
		result.pos = disp.at(1);
		if(disp.size()>2) result.nodePos = disp.at(2);
		
		result.posColor = c.displayColor();	//color of the position
		
		//special color for a traversal node chosen by the user
		if(useTraversal && *it == userChosenId) result.posColor = QString("light green");
		
		//TODO : the value is probably incorrect
		MoveOrdering param = c.getMoveOrdering();
		if(param.checkValue >= 101) {
			//losing child in the previous computation
			result.posColor = QString("blue");
		}
		
		completeResult.push_back(result);
	}
	
	return completeResult;
}

//return the storeId of the node currently computed at a given level of depth
storeId NodeStore::getLevelStoreId(int level) {
	//find the requested (trace) level
	nodeStoreLock.lock();
	list<storeId> listS = NodeStore::getComputingBranch();
	nodeStoreLock.unlock();
	
	list<storeId>::iterator it = listS.begin();
	int i;
	for(i=0; i<level && it!=listS.end(); i++) {
		it++;
	}
	if(it==listS.end()) return 0; //the requested level doesn't exist, 0 by convention
	
	return *it;
}

//get all the information needed for the children trace with only one function
list<NodeTrace> NodeStore::getChildrenTrace(const storeId &id) {
	QMutexLocker locker(&nodeStoreLock);
	warningEnabled=false; //disable the warning messages if we try to access to nodes that don't exist
	list<NodeTrace> completeResult;
	//get the list of children for the requested id
	bool correctId;
	storeIdLink childrenId = NodeStore::getChildrenList(id, correctId);
	if(!correctId) { warningEnabled = true; return completeResult; }
	completeResult = traceFromList(childrenId);
	warningEnabled = true;
	return completeResult;
}

list<NodeTrace> NodeStore::getParentsTrace(const storeId &id) {
	QMutexLocker locker(&nodeStoreLock);
	warningEnabled=false; //disable the warning messages if we try to access to nodes that don't exist
	list<NodeTrace> completeResult;
	//get the list of parents for the requested id
	bool correctId;
	storeIdLink parentId = NodeStore::getParentList(id, correctId);
	if(!correctId) { warningEnabled = true; return completeResult; }
	completeResult = traceFromList(parentId);
	warningEnabled = true;
	return completeResult;
}

//get the information of only one node given in parameter
//we return a list to have compatibility with getParentsTrace and getChildrenTrace
list<NodeTrace> NodeStore::getOneNodeTrace(const storeId &id) {
	QMutexLocker locker(&nodeStoreLock);
	warningEnabled=false; //disable the warning messages if we try to access to nodes that don't exist
	storeIdLink currentId;
	currentId.push_back(id);
	list<NodeTrace> completeResult = traceFromList(currentId);
	warningEnabled = true;
	return completeResult;
}

//no lock, internal function
list<NodeTrace> NodeStore::traceFromList(const storeIdLink &list_id) {
	list<NodeTrace> completeResult;
	bool correctId;
	list<storeId> computingIds = NodeStore::getComputingBranch();
	list<storeId>::iterator computingIt;
	storeIdLink::const_iterator childIt;
	for(childIt=list_id.begin(); childIt!=list_id.end(); childIt++) {
		NodeTrace result;
		result.posColor = QString("transparent"); //background color for the position
		
		BaseNode c = NodeStore::getBaseNode(*childIt, correctId);
		if(!correctId) return completeResult;
		
		//storeid
		result.id = QString("%1").arg(*childIt, 1, 10 );
		
		//number of lives & position
		QStringList disp = c.displayStringList();
		result.nb_lives = disp.at(0);
		result.pos = disp.at(1);
		if(disp.size()>2) result.nodePos = disp.at(2);
		
		//Tree Traversal
		result.traversal_info = QString("");
		result.alive =  0;
		result.bcu =  0;
		result.is_alive = true;
		NodeData d2 = NodeStore::getNodeData(*childIt, correctId);
		if(correctId) {
			//number of still unknown children
			result.alive = d2.unknown_number;
			result.bcu = NodeStore::computeBcu(*childIt);
			
			if(useTraversal) result.traversal_info = d2.traversal_data.getSecondaryTrace();
			
			if(d2.knownState) result.is_alive = false; //already known child
			
			//look for this node in the computing branch : if we are currently computing it, background color is blue
			computingIt = find(computingIds.begin(), computingIds.end(), /*current id :*/ *childIt);
			if(computingIt!=computingIds.end()) result.posColor = QString("blue");
			
			//special case of a traversal node chosen by the user
			if(useTraversal && *childIt == userChosenId) result.posColor = QString("light green");
		}
		
		completeResult.push_back(result);
	}
	
	return completeResult;
}

//debug
//return a string with all the database sizes
string NodeStore::storeSize() {
	string result("Size ");
	result += QString("id:%1 ").arg(idMap.size()).toStdString();
	result += QString("node:%1 ").arg(nodeDataBase.size()).toStdString();
	return result;
}

//debug : function to check the consistency of some data
//in particular, check that unknown_number is always equal to the number of unknown children
void NodeStore::checkConsistency(const storeId &id) {
	QMutexLocker locker(&nodeStoreLock);
	bool correctId; //unused
	storeIdLink children = getChildrenList(id, correctId);
	
	storeIdLink::iterator it;
	int unknownTotal=0;
	for(it=children.begin(); it!=children.end(); it++) {
		NodeData data = getNodeData(*it, correctId);
		if(!data.knownState) unknownTotal++;
	}
	
	NodeData data = getNodeData(id, correctId);
	//note : an unknownTotal of 0 is not incorrect, it corresponds to terminal nodes in some tree traversal
	if(data.unknown_number!= unknownTotal && unknownTotal!=0) {
		cout << "Incorrect unknown number of children. unknown_number=" << data.unknown_number <<
		        " real_unknown=" << unknownTotal << " id=" << id << endl;
	}
}
