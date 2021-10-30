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
#include "computationthread.h"
#include "../gameglobal.h"
#include "database.h"
#include "trace.h"

//TODO : there is probably some nodes that are not deleted in multi-steps computations

//main function of the program
//compute the value of ComputationThread::BaseNodeIni with trace/nodestore
//it is essentially the same function as ComputationThread::recursiveLoop, but much more complicated
void NodeStore::mainLoop() {
	//add BaseNodeIni to NodeStore and to the computing branch
	mainloopCount = 0;
	TRnodesCount = 0;
	DFnodesCount = 0;
	maxNodesCount = 0;
	rootId = addNode(ComputationThread::BaseNodeIni);
	compBranchAdd(rootId);
	
	bool resultKnown;
	bool correctId;
	bool nodeFinished = false;
	storeId parentId;
	
	//create the internal object for "static" calls to the travesal algorithm
	if(Parameter::isCheck) {
		//in case of a check computation, use an alpha-beta algorithm (= no traversal)
		BaseTraversal bt(BaseTraversal::noTraversalIndex());
		traversalStatic = bt;
	} else {
		//in case of a normal computation, use the choice of the user
		BaseTraversal bt(Parameter::traversalChoice);
		traversalStatic = bt;
	}
	traversalStatic.initTraversalStatic();	//init the object
	
	//loop until the value of the firstId is known
	while(true) { //infinite loop : test is done inside the loop (computingBranch.size()==0)
		//stop computation if asked by user
		if (Trace::stop_signal()) return;
		mainloopCount++;
		
		//-----------compute the last id of the computing branch------------
		storeId lastId = compBranchLast();
		if(debugLevel==4) {
			cout << "Start computation of #" << lastId << endl;
			cout << storeSize() << endl;
		}
		
		BaseNode currentNode = externGetBaseNode(lastId, correctId);
		NodeData currentNodeData = externGetNodeData(lastId, correctId);
		
		resultKnown = currentNode.tryImmediateResult();
		updNode(lastId, currentNode); //the representation may have changed in tryImmediateResult
		
		list<BaseNode> NClist;
		if(!resultKnown) {
			//if the node does not exist, compute its children
			resultKnown = currentNode.computeNodeChildren(NClist);
			updNode(lastId, currentNode); //the representation may have changed in computeNodeChildren
			
			//if the node is unknown and there is no children,
			//then it means that all children were "useless" for computing the final result
			if(!resultKnown && NClist.empty()) {
				currentNode.resultFromAllChildren(NClist);
				resultKnown = true;
			}
			
			if(!resultKnown) {
				//WARNING : in the case of complex algorithm
				//it is possible that the children of the node have ALREADY been stored
				//in that case, they should of course not be stored again.
				if(currentNodeData.children_number==0) {
					//if the node is still unknown, add the children to NodeStore
					storeIdLink childrenId = addNodeList(NClist);
					
					//add the link between currentNode and the children
					createLinks(lastId, childrenId);
					
					//update nodeData
					currentNodeData.children_number=NClist.size();
					currentNodeData.unknown_number=currentNodeData.children_number;	//number of still alive children
					setNodeData(lastId, currentNodeData);
				}
			}
		}
		
		//----------update the NodeStore structure with the result (if there is one)--------
		//TODO : this part will become an independent recursive function
		if(resultKnown) {
			//cout << "A " << lastId << endl;
			bool endLoop=false;
			
			while(!endLoop) {
				//cout << "B " << lastId << endl;
				endLoop = true;
				currentNodeData.knownState = true;
				setNodeData(lastId, currentNodeData);
				
				nodeFinished = currentNode.resultIsComplete();
				updNode(lastId, currentNode); //the representation may have changed in resultIsComplete
				if(nodeFinished) {
					if(useTraversal) traversalStatic.traversalNodeFinished(lastId);
					
					compBranchDel(); //computation of currentNode is finished
					
					//if the computing branch is empty, the computation is finished
					if(compBranchIsEmpty()) {
						//copy the current node in ComputationThread, so that ComputationThread can access to the result
						ComputationThread::BaseNodeIni = currentNode;
						return;
					}
					
					//try to deduce the result of the parent
					parentId = compBranchLast();
					BaseNode parentNode = externGetBaseNode(parentId, correctId);
					NodeData parentNodeData = externGetNodeData(parentId, correctId);
					
					parentNodeData.unknown_number--;
					setNodeData(parentId, parentNodeData);
					
					if(parentNode.resultFromOneChild(currentNode)) {
						endLoop = false;
					} else {
						//if all children are computed, deduce the global result
						if(parentNodeData.unknown_number==0) {
							list<BaseNode> children = getChildrenBaseNode(parentId , correctId);
							parentNode.resultFromAllChildren(children);
							endLoop = false;
						}
					}
					
					if(!endLoop) {
						//parentNode is deduced : delete the children
						deleteChildrenLink(parentId);
						
						//same process again with parentNode
						currentNode = parentNode;
						currentNodeData = parentNodeData;
						lastId = parentId;
					}
				}
			}
		}
		
		//-------choose the next Node to compute and add it at the end of the computing branch----
		//simple algorithm : choose the next unknown child of the last node of the computing branch
		if(!resultKnown || (resultKnown && nodeFinished)) {
			storeId chosenId;
			int val=Trace::user_request();
			if(val==-5) {
				return; //end of computation
			}
			
			bool idIsChosen = false;
			if(useTraversal) {
				idIsChosen = traversalStatic.traversalIteration();
			} else {
				//check if the user has requested a specific ID
				storeId userId = Trace::checkRequestedId();
				if(userId!=-1) {
					createCompBranchTo(userId);
					idIsChosen = true;
				}
			}
			
			if(!idIsChosen) {
				//move ordering algorithm only, with user interaction
				if(val==-4) {
					chosenId = nextUnknownChild(); //no signal from the user : choose the next unknown child
				} else {
					chosenId = userRequestId(val); //choose the child indicated by the user
				}
				
				parentId = computingBranch.back();
				compBranchAdd(chosenId);
			}
		} else {
			if(debugLevel==4) {
				cout << "Next step in the same Node " << endl;
			}
			
			//a step in a multi-step algorithm has finished
			//init again the nodeData of the node
			NodeData d = externGetNodeData(lastId, correctId);
			d.initAgain();
			setNodeData(lastId, d);
		}
	}
}

//user-signal reception
storeId NodeStore::userRequestId(int val) {
		//---------go back to the correct level------------------------
		if(val==-1) {	//zapping above
			while(val==-1) {
				//printList(computingBranch);
				compBranchDel();
				deleteChildrenLink(lastComputedId);
				val=Trace::user_request();
			}
		}
		
		//--------action requested for the current level---------------
		if (val==-2) {	//simple zapping at this level
			return nextUnknownChild();
		} else {	//zapping backward (val=-3)
			return previousUnknownChild();
		}
}

//next child of the last id
storeId NodeStore::nextUnknownChild() {
	QMutexLocker locker(&nodeStoreLock);
	
	if(computingBranch.empty()) {
		cout << "error in NodeStore::nextUnknownChild() : computing branch is empty";
		return 0;
	}
	
	bool correctId; //unused
	storeId parentId = computingBranch.back();
	storeIdLink childrenIdList = getChildrenList(parentId , correctId);
	storeIdLink::iterator it=childrenIdList.begin();
	
	//if the lastComputedId is valid, iterate until it is found
	if(lastComputedId != -1) {
		bool lastIdFound = false;
		for( ; it!=childrenIdList.end() && !lastIdFound; it++) {
			if(*it==lastComputedId) {
				lastIdFound = true;
			}
		}
	}
	
	//find the next unknown child from the current position
	bool isKnown = true;
	storeId chosenId=0;
	for( ; it!=childrenIdList.end() && isKnown; it++) {
		chosenId=*it;
		NodeData childrenData = getNodeData(chosenId, correctId);
		isKnown = childrenData.knownState;
	}
	
	//if no unknown child was found, try again from the beginning of the list
	if(isKnown) {
		for(it=childrenIdList.begin(); it!=childrenIdList.end() && isKnown; it++) {
			chosenId=*it;
			NodeData childrenData = getNodeData(chosenId, correctId);
			isKnown = childrenData.knownState;
		}
	}
	
	if(isKnown) cout << "error in NodeStore::nextUnknownChild : all children are already known" << endl;
	if(debugLevel==4) cout << "nextUnknownChild : parent #" << parentId << " child #" << chosenId << " state " << isKnown << endl;
	
	return chosenId;
}

//previous child of the last id
//this is exactly the same function as nextUnknownChild but with a reverse_iterator
storeId NodeStore::previousUnknownChild() {
	QMutexLocker locker(&nodeStoreLock);
	
	if(computingBranch.empty()) {
		cout << "error in NodeStore::previousUnknownChild() : computing branch is empty";
		return 0;
	}
	
	bool correctId; //unused
	storeId parentId = computingBranch.back();
	storeIdLink childrenIdList = getChildrenList(parentId , correctId);
	storeIdLink::reverse_iterator it=childrenIdList.rbegin();
	
	//if the lastComputedId is valid, iterate until it is found
	if(lastComputedId != -1) {
		bool lastIdFound = false;
		for( ; it!=childrenIdList.rend() && !lastIdFound; it++) {
			if(*it==lastComputedId) {
				lastIdFound = true;
			}
		}
	}
	
	//find the next unknown child from the current position
	bool isKnown = true;
	storeId chosenId=0;
	for( ; it!=childrenIdList.rend() && isKnown; it++) {
		chosenId=*it;
		NodeData childrenData = getNodeData(chosenId, correctId);
		isKnown = childrenData.knownState;
	}
	
	//if no unknown child was found, try again from the beginning of the list
	if(isKnown) {
		for(it=childrenIdList.rbegin(); it!=childrenIdList.rend() && isKnown; it++) {
			chosenId=*it;
			NodeData childrenData = getNodeData(chosenId, correctId);
			isKnown = childrenData.knownState;
		}
	}
	
	if(isKnown) cout << "error in NodeStore::previousUnknownChild : all children are already known" << endl;
	if(debugLevel==4) cout << "previousUnknownChild : parent #" << parentId << " child #" << chosenId << " state " << isKnown << endl;
	
	return chosenId;
}
