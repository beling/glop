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

#ifndef TRAVERSAL_H
#define TRAVERSAL_H

#include "../gameglobal.h"
#include "../global.h"
#include "basenode.h"
#include "interface.h"

#include <QString>

typedef int storeId;

//! @defgroup TraversalGroup Traversal

//! Base class for defining a new kind of Traversal algorithm
//! A new traversal algorithm is defined by deriving a new class from this BaseTraversal class
//! In order to create correctly the clone, register and access mecanism, the following steps are needed :
//! 1. The macro TRAVERSAL_REGISTER_MACRO(NewTRaversalClass) must be added in the public part of derived class 
//! 2. NewTRaversalClass NewTRaversalClass::root(0); must be added at the beginning of the .cpp file of the new class
//! In 1. and 2. the parameter "NewTRaversalClass" must be replaced by the real name of the new class
//! Then, the virtual functions of BaseTraversal must be reimplemented in the new class
//! @ingroup TraversalGroup
class BaseTraversal {
  private:
	BaseTraversal * p_baseTraversal;
	static vector<BaseTraversal *>& derivedClassList();
	void initializePointer(int classIndex);
	
  public:
	//register mecanism
	static int registerNewClass(BaseTraversal *a);  //!< add a new traversal algorithm node to the list of available algorithms
	static int classesNumber();
	static int& noTraversalIndex();
	bool validPointer() const;
	BaseTraversal * getPointer() const;
	BaseTraversal();
	BaseTraversal(int classIndex);
	BaseTraversal(const BaseTraversal& a);
	BaseTraversal& operator=(const BaseTraversal& a);
	virtual ~BaseTraversal();
	virtual BaseTraversal* clone() const;
	
	//functions that should be reimplemented in all traversal algorithms
	//note : these functions are only a first experimental set
	virtual void init();						//init the values of the traversal variables
	virtual void initAgain();					//init again the values of the traversal variables (case of multi-step algorithms)
	virtual void initFromNode(BaseNode& a);		//init the traversal variables from a given node
	virtual bool isTraversalNode();				//indicate if the node is a traversal node or not
	virtual QString getMainTrace();				//return a string for the main trace
	virtual QString getSecondaryTrace();		//return a string for the secondary trace
	
	//functions that should be reimplemented in all algorithms
	//note : these functions must be understood from a "static" viewpoint
	//they are called from a real object (and not with a static call) only in order to derivate the call to the reimplemented function 
	virtual void traversalNodeFinished(const storeId &id);
	virtual void initTraversalStatic();
	virtual bool traversalIteration();
	virtual Interface getParamDef();
};

//the following macro must be included in the public section of any class derived from Traversal
//It provides the clone, access and register mecanism for the derived traversal algorithm
#define TRAVERSAL_REGISTER_MACRO(TRAVERSAL_NAME) \
	TRAVERSAL_NAME() {}; \
	virtual BaseTraversal* clone() const { return new TRAVERSAL_NAME(*this); }; \
	static TRAVERSAL_NAME * access(BaseTraversal &nc) { return (TRAVERSAL_NAME *) nc.getPointer(); }; \
	TRAVERSAL_NAME(int a) { registeredIndex() = BaseTraversal::registerNewClass(this); \
						    if(a==-1) noTraversalIndex() = registeredIndex(); \
						  }; \
	static TRAVERSAL_NAME root; \
	static int& registeredIndex() { static int index; return index; };

//------------- Special "No traversal" algorithm ------------------

class NoTraversal : public BaseTraversal {
  public:
	TRAVERSAL_REGISTER_MACRO(NoTraversal)
	
	//this is the only function that is authorized to be called for this traversal
	virtual void initTraversalStatic(); 	//init the parameters of NoTRaversal at the start of mainloop
	virtual Interface getParamDef();
	
	//all the other functions don't need any special reimplementation
	virtual void init() {};
	virtual void initAgain() {};
	virtual void initFromNode(BaseNode& /*a*/) {};
	virtual bool isTraversalNode() {return false;};
	virtual QString getMainTrace() {QString a; return a;};
	virtual QString getSecondaryTrace() {QString a; return a;};
	virtual void traversalNodeFinished(const storeId &/*id*/) {};
	virtual bool traversalIteration() {return false;};
};

#endif
