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

#include "traversal.h"
#include "../computation/nodestore.h"

vector<BaseTraversal *>& BaseTraversal::derivedClassList() {
	static vector<BaseTraversal *> internalList;
	return internalList;
}

//add a new traversal to the traversal list
//return the identifier for this traversal
int BaseTraversal::registerNewClass(BaseTraversal *a) {
	derivedClassList().push_back(a);
	
	//return the index in the list
	return (derivedClassList().size() - 1);
}

int BaseTraversal::classesNumber() {
	return (int) (derivedClassList().size());
}

//used to remember what is the index of the particular NoTraversal (=alpha-beta traversal) object
int& BaseTraversal::noTraversalIndex() {
	static int index;
	return index;
}

void BaseTraversal::initializePointer(int classIndex) {
	//initialize the internal pointer with the correct traversal object
	if(classIndex<(int)derivedClassList().size()) { //security
		//create a new object of the correct traversal by cloning
		p_baseTraversal = (derivedClassList()[classIndex])->clone();
	} else {
		cout << "BaseTraversal error : unknown traversal" << endl;
		p_baseTraversal = 0;
	}
}

BaseTraversal::BaseTraversal() {
	p_baseTraversal=0;
}

BaseTraversal::BaseTraversal(int classIndex) {
	initializePointer(classIndex);
}

BaseTraversal::~BaseTraversal() {
	if(p_baseTraversal!=0) {
		delete p_baseTraversal;
		p_baseTraversal=0;
	}
}

//we need to copy the object pointed by p_baseTraversal and not the pointer itself
BaseTraversal::BaseTraversal(const BaseTraversal& a) {
	if(a.p_baseTraversal != 0) {
		p_baseTraversal = a.p_baseTraversal->clone();
	} else {
		p_baseTraversal = 0;
	}
}

BaseTraversal& BaseTraversal::operator=(const BaseTraversal& a) {
	//protection against memory leak in case of code like n = n
	if(this != &a) {
		//delete the old p_baseTraversal
		if(p_baseTraversal!=0) {
			delete p_baseTraversal;
			p_baseTraversal=0;
		}
		
		if(a.p_baseTraversal != 0) {
			p_baseTraversal = a.p_baseTraversal->clone();
		} else {
			p_baseTraversal = 0;
		}
	}
	
	return *this;
}

//note:this function is never called (always reimplemented in the derived class)
BaseTraversal* BaseTraversal::clone() const {
	return new BaseTraversal(*this);
}

//return true if the pointer is valid (not null)
bool BaseTraversal::validPointer() const {
	if(p_baseTraversal==0) {
		cout << "BaseTraversal::p_baseTraversal is Null. Segmentation Fault is going to occur." << endl;
		return false;
	} else {
		return true;
	}
}

BaseTraversal * BaseTraversal::getPointer() const {
	validPointer();
	return p_baseTraversal;
}

//-------------- virtual functions that need to be reimplemented ---------

void BaseTraversal::init() {
	validPointer();
	p_baseTraversal->init();
}

void BaseTraversal::initAgain() {
	validPointer();
	p_baseTraversal->initAgain();
}
void BaseTraversal::initFromNode(BaseNode& a) {
	validPointer();
	return p_baseTraversal->initFromNode(a);
}

bool BaseTraversal::isTraversalNode() {
	validPointer();
	return p_baseTraversal->isTraversalNode();
}

QString BaseTraversal::getMainTrace() {
	validPointer();
	return p_baseTraversal->getMainTrace();
}

QString BaseTraversal::getSecondaryTrace() {
	validPointer();
	return p_baseTraversal->getSecondaryTrace();
}

void BaseTraversal::traversalNodeFinished(const storeId &id) {
	validPointer();
	p_baseTraversal->traversalNodeFinished(id);
}

//note about NodeStore : for all traversals, useTraversal is initialized to true
//it means that NodeStore will call the traversal algorithm
// The only exception is the special NoTraversal class, which corresponds to the use of no specific traversal
void BaseTraversal::initTraversalStatic() {
	validPointer();
	NodeStore::useTraversal = true; //initialize NodeStore so that Traversal is used
	p_baseTraversal->initTraversalStatic();
}

bool BaseTraversal::traversalIteration() {
	validPointer();
	return p_baseTraversal->traversalIteration();
}

Interface BaseTraversal::getParamDef() {
	validPointer();
	return p_baseTraversal->getParamDef();
}

//----------------------------- Special NoTraversal class ------------
//register mecanism
//-1 is used to indicate that this class is the NoTraversal class
NoTraversal NoTraversal::root(-1);

//indicates that for this kind of traversal, NodeStore should do no call to the traversal algorithm
//this initialization is the only call done in NodeStore
void NoTraversal::initTraversalStatic() {
	NodeStore::useTraversal = false;
}

Interface NoTraversal::getParamDef() {
	Interface result;
	result.name="Alpha-beta";
	result.addLabel("No particular parameters.", /*pos*/ 0, 0);
	return result;
}