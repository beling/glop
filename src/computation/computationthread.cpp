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

#include "computationthread.h"
#include "trace.h"
#include "nodestore.h"

#include <time.h>

BaseNode ComputationThread::BaseNodeIni;

//launch a computation
//this function needs to be called through the Qt special function start()
void ComputationThread::run() {
	//initialisation so that rand() won't give always the same values
	//note : the initializaion for rand() is needed for each thread on some platforms
	srand(time(NULL));	
	
	//start a computation
	startOneComputation();
	
	//in case of random check cyle, perform the computation until all the cycles have been done
	if(Parameter::isCheck) {
		if(Parameter::RandomCheckCycles>0) Parameter::RandomCheckCycles--;
		while(Parameter::RandomCheckCycles>0 && !Trace::stop_signal()) {
			BaseNode node(Parameter::nodeChoice, Parameter::computation_kind);
			node.prepareNextCycle(); //move the check base in the normal base
			startOneComputation();
			Parameter::RandomCheckCycles--;
		}
	}
	
	//computation is finished. Result is saved in resultString
	compthread_mutex.lock();
		computationRunning = false;
	compthread_mutex.unlock();
}

//start a computation and clean the objects when finished
void ComputationThread::startOneComputation() {
	//initialize starting position
	BaseNode coreIni(Parameter::nodeChoice, Parameter::computation_kind);
	coreIni.initRootNode(); //initialize the parameters of the first BaseNode
	BaseNodeIni = coreIni;
	
	if(recursiveStructure) {
		do {
			recursiveLoop(BaseNodeIni);
		} while(!BaseNodeIni.resultIsComplete() && !Trace::stop_signal());
	} else {
		NodeStore::clearNodes();				//just to be sure all is correctly initialized
		NodeStore::mainLoop();
		NodeStore::clearNodes(); //clear the nodes at the end of the computation
	}
	
	//Save the result in resultString
	compthread_mutex.lock();
		BaseNode::computationResult = BaseNodeIni.resultString();
	compthread_mutex.unlock();
}

//compute the value of the given BaseNode with a simple recursive algorithm
//it is used to check computations without the complicated code of the trace/nodestore
void ComputationThread::recursiveLoop(BaseNode& node) {
	if(node.tryImmediateResult()) return;		//check that the node is not already known
	
	//compute children
	list<BaseNode> nodeChildren;
	if(node.computeNodeChildren(nodeChildren)) return;
	
	//----- recursive computation on children---------------------------
	list<BaseNode>::iterator child;
	for(child=nodeChildren.begin(); child!=nodeChildren.end(); child++) {
		do {
			recursiveLoop(*child);
			if (Trace::stop_signal()) return; //stop computation if asked by user
		} while(!child->resultIsComplete());
		
		//try to deduce the result of computation
		if(node.resultFromOneChild(*child)) return;
	}
	node.resultFromAllChildren(nodeChildren);
}
