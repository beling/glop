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
#include "classicpnsearch.h"

#include <QMutex>

//register mecanism
ClassicPN ClassicPN::root(0);

//---------------------------------PN Search---------------------------------------

//! Define the interface for parameters of Pn-Search
Interface ClassicPN::getParamDef() {
	Interface result;
	result.name="PN-Search";
	result.addCheckButton("Use PN-heuristic", false, 0, 0);
	result.link(ClassicPN::usePNheuristic, "useHeuristic");
	return result;
}

//initialize the pn variables for a new node
void ClassicPN::init() {
	proof=1;
	disproof=1;
	PNnode=0;		//at start, the node has never been chosen by the PN
	subTreeSize=1;	//at start, the subtree consists of only the current node
	subTreeDepth=0;	//the node has no subtree
}

//init again after a step has finished inside a multi-step algorithm (typically 0, 1, 2 algo of nimber)
void ClassicPN::initAgain() {
	init();
}

//initialize the pn variables for a specific node
//here, we can change the default values of init()
//for example by taking into account the number of children or some heuristic on the difficulty of the node
void ClassicPN::initFromNode(BaseNode& a) {
	//initialization with heuristics representing the difficulty of proving/deproving the node
	//the node will call in turn the heuristics for the current game 
	if(usePNheuristic) {
		unsigned int heuristic = a.traversalHeuristic();
		proof = heuristic;
		disproof = heuristic;		//initialize the disproof value with the heuristic for the node
	}
}

//A PN-search node is always a traversal one
bool ClassicPN::isTraversalNode() {
	return true;
}

//return a string for the main trace
QString ClassicPN::getMainTrace() {
	QString	result = QString("(%1;%2)").arg(proof, 1, 10 ).arg(disproof, 1, 10 ); //display p and d values
	result += QString(" - %1").arg(subTreeSize, 1, 10);
	return result;
}

//return a string for the children trace
QString ClassicPN::getSecondaryTrace() {
	QString DisproofNb;
	if(disproof==-1){
		DisproofNb=QString("inf");
	} else {
		DisproofNb=QString("%1").arg(disproof, 1, 10 );
	}
	
	QString result = QString("(%1;").arg(proof, 1, 10 ) + DisproofNb + QString(")"); //display p and d values
	result += QString(" - %1").arg(subTreeSize, 1, 10);
	result += QString(" - %1").arg(subTreeDepth, 1, 10);
	return result;
}

//update the variables after a node has been proved
void ClassicPN::proved() {
	proof = 0;      //nothing to do to prove the node
	disproof = -1;  //inf, the node is impossible to disprove
	subTreeSize = 1; //the children will be deleted, but not the node itself
	subTreeDepth = 0;
}

//update the variables after a node has been disproved
void ClassicPN::disproved() {
	proof = -1;    //inf, the node is impossible to prove
	disproof = 0;  //nothing to do to disprove the node
	subTreeSize = 1; //the children will be deleted, but not the node itself
	subTreeDepth = 0;
}

//----------------------- Static functions ----------------

bool ClassicPN::usePNheuristic=false;
storeId ClassicPN::PNChosenId=0;

//init the parameters of PN at the start of mainloop
//warning : nodestoreLock seems not needed because we modify only pn parameters
void ClassicPN::initTraversalStatic() {
	//the root node is a permanent PN-node from the start
	bool correctId;
	storeId rootId = NodeStore::getRootId();
	NodeData rootNodeData = NodeStore::externGetNodeData(rootId, correctId);
	access(rootNodeData.traversal_data)->PNnode = 2;
	NodeStore::setNodeData(rootId, rootNodeData);
	NodeStore::userChosenId = -1;
}

//return true if PN iteration has chosen the next id
bool ClassicPN::traversalIteration() {
	bool correctId;
	
	storeId userId = Trace::checkRequestedId(); //check if the user has chosen an id
	if(userId!=-1) {
		NodeStore::userChosenId = userId;
		PN_initChosenId();
	}
	
	//---- [PNsearch] ---
	//choose the next PN-node when we have done the limit number of depth-first iterations
	storeId branchLastId = NodeStore::compBranchLast();
	updatePropagatePN(branchLastId);	//update the PN-values from the last id
	choosePNBranch();				//choose the computing branch (from root) with PN-values
	storeId chosenPNId = NodeStore::compBranchLast();
	NodeData chosenPNData = NodeStore::externGetNodeData(chosenPNId, correctId);
	return true;
}

//update the p/d values from the children, and propagate to the parent
void ClassicPN::updatePropagatePN(const storeId &id) {
	QMutexLocker locker(&NodeStore::nodeStoreLock);
	internalUpdatePropagatePN(id);
}

//internal function without lock to allow recursive calls
//update the proof and disproof value of a node from the values of the children
//proof value = min of children disproof values, -1 = inf
//disproof value = sum of children proof values, -1 = inf
//note : in this draft, we go back to the root,
//it would be better to stop when the values are left unchanged
void ClassicPN::internalUpdatePropagatePN(const storeId &id) {
	bool correctId; //not used
	
	NodeData data = NodeStore::getNodeData(id, correctId);  //nodeData
	if(access(data.traversal_data)->proof != -1 && access(data.traversal_data)->disproof != -1 && data.children_number!=0) {
		storeIdLink childrenList = NodeStore::getChildrenList(id, correctId);
		storeIdLink::iterator Ci; //child_i
		
		access(data.traversal_data)->proof  = -1; //inf
		access(data.traversal_data)->disproof = 0;
		access(data.traversal_data)->subTreeSize=1;
		access(data.traversal_data)->subTreeDepth=0;
		for(Ci=childrenList.begin(); Ci!=childrenList.end(); Ci++) {
			NodeData childData = NodeStore::getNodeData(*Ci, correctId);
			
			//update the PN values
			//-------------------update proof value----------------
			if(access(data.traversal_data)->proof != -1) {
				//the child disproof is better if it is less than the current value, and different from inf
				if( (access(childData.traversal_data)->disproof != -1) && (access(childData.traversal_data)->disproof < access(data.traversal_data)->proof) ) { 
					access(data.traversal_data)->proof = access(childData.traversal_data)->disproof;  
				}
			} else {
				access(data.traversal_data)->proof = access(childData.traversal_data)->disproof; //any value is less than inf
			}
			
			//-------------------update disproof value--------------
			if(access(data.traversal_data)->disproof != -1) {
				if( (access(childData.traversal_data)->proof != -1) ) {
					access(data.traversal_data)->disproof += access(childData.traversal_data)->proof;
				} else {
					access(data.traversal_data)->disproof = -1; //a child has an infinite proof number, so the disproof number will be infinite as well
				}
			}
			
			//----------------update subtree size and depth-------------------
			//add the number of children only if it is a permanent PN node , i.e PNnode>1
			//(if the node is not permanent, the children will be deleted in choosePNBranch)
			if(access(data.traversal_data)->PNnode>1) {
				access(data.traversal_data)->subTreeSize += access(childData.traversal_data)->subTreeSize; //subtree size = Sum of the subtree size of the children
				
				if(access(childData.traversal_data)->subTreeDepth>=access(data.traversal_data)->subTreeDepth) {
					access(data.traversal_data)->subTreeDepth = access(childData.traversal_data)->subTreeDepth+1;
				}
			}
		}
		
		//update the nodeData (similar to setNodeData without the lock)
		NodeStore::nodeDataBase[id].storedNodeData = data;
	}
	
	if(NodeStore::debugLevel==4) cout << "(UpdatePropagatePN) #" << id << " p:" << access(data.traversal_data)->proof << " d:" << access(data.traversal_data)->disproof << endl;
	
	//recursive call to update all the parents (except if we have gone back to the root id)
	//TODO : call also other parents (and not only the first one) if transpositions are implemented
	if(id != NodeStore::rootId) {
		storeIdLink parentList = NodeStore::getParentList(id, correctId);
		if(!parentList.empty()) {
			internalUpdatePropagatePN(parentList.front());
		}
	}
}

//choose a branch to compute
//note : in this draft, we start from the root
//it would be better to start from the last point with unchanged p/d values
void ClassicPN::choosePNBranch() {
	bool correctId; //not used
	//---------clear the current computing branch except root node--------
	//but stop if we are computing an id chosen by the user
	int branchSize=NodeStore::compBranchSize();
	
	int i;
	bool end = false;
	for(i=0; i<branchSize-1 && !end; i++) {
		storeId branchEndId = NodeStore::compBranchLast(); //last element
		
		if(branchEndId!=NodeStore::userChosenId) {
			NodeStore::compBranchDel();
			
			//if the node has not been chosen at least two times by the PN algorithm, delete its children
			NodeData data = NodeStore::externGetNodeData(NodeStore::lastComputedId, correctId);
			if(access(data.traversal_data)->PNnode < 2) NodeStore::deleteChildrenLink(NodeStore::lastComputedId);
		} else {
			//stop here if we have come back to the id chosen by the user
			end = true;
		}
	}
	
	//--------create a new computing branch----------
	storeId id = NodeStore::compBranchLast();
	id = choosePNNextId(id); //find the next id after the last id of the computing branch
	while(id != -1) {
		//add the chosen id to the computing branch
		NodeStore::compBranchAdd(id);
		PNChosenId = id;
		
		//indicate in the node data that it has been chosen as a PN node
		//it will prevent the node children to be deleted until the node is computed or useless
		NodeData data = NodeStore::externGetNodeData(id, correctId);
		
		//if the node has been chosen less than three times, increment the number of times it has been chosen
		if(access(data.traversal_data)->PNnode < 3) {
			access(data.traversal_data)->PNnode++;
			NodeStore::setNodeData(id, data);
		}
		
		//if the node has been chosen less than three times (after incrementation), this is the end of the PN-branch
		if(access(data.traversal_data)->PNnode < 3) {
			id =-1;
		} else {
			//find the next id (the child with the least disproof value)
			id = choosePNNextId(id);
		}
	}
}

//return the nextId in the PN branch after a given id
//return -1 if there is no nextId
storeId ClassicPN::choosePNNextId(const storeId &id) {
	QMutexLocker locker(&NodeStore::nodeStoreLock);
	storeId nextId = -1;  //best child found
	bool correctId;       //not used
	NodeData data = NodeStore::getNodeData(id, correctId);  //nodeData
	if(data.children_number==0) return nextId; //return -1 if there are no children
	
	storeIdLink childrenList = NodeStore::getChildrenList(id, correctId);
	storeIdLink::iterator Ci;
	
	//we try to prove the node
	//look for the least disproof number, but not inf (already disproved)
	//note : there should be no child with d=0, since the current node would be already proved
	int min =  -1;        //min proof or disproof value found among the children
	for(Ci=childrenList.begin(); Ci!=childrenList.end(); Ci++) {
		NodeData childData = NodeStore::getNodeData(*Ci, correctId);
		if( (access(childData.traversal_data)->disproof!=-1) && ((min == -1) || (access(childData.traversal_data)->disproof < min ) ) ) {
			//any value is better than inf, or a value less than min, except inf (-1)
			min = access(childData.traversal_data)->disproof;
			nextId = *Ci;
		}
	}
	
	if(NodeStore::debugLevel==4) cout << "(choosePNNextId) #" << id << " nextId: #" << nextId <<endl;
	return nextId;
}

//This function is called after the node has been proved or disproved
//We don't need to distinguish the two cases, because in case of a disproved node,
//   mainloop will deduce the result of the parent and the current node will be deleted
//For this reason, we always consider only proved node
//TODO : for sake of clarity, but it would be better to distinguish the proved and disproved cases 
void ClassicPN::traversalNodeFinished(const storeId &id) {
	provedNode(id); //the node has been proved (winning)
}

//set the proof/disproof values when the node has been proved
void ClassicPN::provedNode(const storeId &id) {
	if(NodeStore::debugLevel==4) cout << "(provedNode) #" << id << " p:" << 0 << " d:" << -1 << endl;
	
	bool correctId;       //not used
	NodeData data = NodeStore::externGetNodeData(id, correctId);  //nodeData
	access(data.traversal_data)->proved();
	NodeStore::setNodeData(id, data);
}

//set the proof/disproof values when the node has been disproved
void ClassicPN::disprovedNode(const storeId &id) {
	if(NodeStore::debugLevel==4) cout << "(disprovedNode) #" << id << " p:" << -1 << " d:" << 0 << endl;
	
	bool correctId;       //not used
	NodeData data = NodeStore::externGetNodeData(id, correctId);  //nodeData
	access(data.traversal_data)->disproved();
	NodeStore::setNodeData(id, data);
}

//init the computing branch with the correct ids to reach the requested chosen id
void ClassicPN::PN_initChosenId() {
	//find the correct list of nodes that go from the root to the requested id
	NodeStore::nodeStoreLock.lock();
	storeIdLink parentList;
	list<storeId> newComputingBranch;
	bool correctId;
	
	//check that the requested node is not already computed
	//correctId is set to false if the node is already computed (and we will do nothing)
	NodeData requestedNodeData = NodeStore::getNodeData(NodeStore::userChosenId, correctId);
	if(requestedNodeData.knownState) correctId = false;
	
	storeId previousId=NodeStore::userChosenId; // start from the id requested by the user
	while(correctId && previousId!=NodeStore::rootId) {
		newComputingBranch.push_front(previousId);
		parentList = NodeStore::getParentList(previousId, correctId);
		if(correctId && !parentList.empty()) {
			previousId = parentList.front(); //any parent is ok, so choose the first one
		} else {
			correctId = false;
		}
	}
	NodeStore::nodeStoreLock.unlock();
	
	//copy the new branch in the normal branch if we found a path to the root
	//we do nothing if we have not found a correct path
	if(correctId) {
		//---------clear the current computing branch except the root node ------
		int branchSize=NodeStore::compBranchSize();
		
		int i;
		for(i=0; i<branchSize-1; i++) {
			NodeStore::compBranchDel();
			
			//if the node has not been chosen at least two times by the PN algorithm, delete its children
			NodeData data = NodeStore::externGetNodeData(NodeStore::lastComputedId, correctId);
			if(access(data.traversal_data)->PNnode < 2) NodeStore::deleteChildrenLink(NodeStore::lastComputedId);
		}
		
		//check that all nodes are still correct
		list<storeId>::iterator Ni;
		correctId=true;
		for(Ni=newComputingBranch.begin(); Ni!=newComputingBranch.end() && correctId; Ni++) {
			NodeData data = NodeStore::externGetNodeData(*Ni, correctId);
		}
		
		//------add the new computing branch
		if(correctId) {
			for(Ni=newComputingBranch.begin(); Ni!=newComputingBranch.end(); Ni++) {
				NodeStore::compBranchAdd(*Ni);
			}
		} else {
			//some node in the path are invalid : add only the root node
			NodeStore::compBranchAdd(NodeStore::rootId);
		}
	}
}
