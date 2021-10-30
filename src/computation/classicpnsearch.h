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

#ifndef CLASSICPNSEARCH_H
#define CLASSICPNSEARCH_H

#include "../baseclass/traversal.h"

//Class for the classic PN-search (without improvements)
//This class representents the traversal variables stored in each node of the tree
//Notes : proof, disproof and subTreeSize are coded as int, because all these numbers can become bigger than 32767
//        PNnode and subTreeDepth are coded as short int, because it is virtually impossible that they reach 32767
class ClassicPN : public BaseTraversal {
  public:
	TRAVERSAL_REGISTER_MACRO(ClassicPN)
	
	//--------reimplement the virtual functions of BaseTraversal--------
  	virtual void init();  //init the values of the traversal variables
	virtual void initAgain();
	virtual void initFromNode(BaseNode& a); //init the traversal variables from a given node
	virtual bool isTraversalNode();  //indicate if the node is a traversal node or not
	virtual QString getMainTrace(); //return a string for the main trace
	virtual QString getSecondaryTrace(); //return a string for the secondary trace
	
	//---- parameters specific to PNsearch ---
	int proof;      //proof value, -1 = infinite
	int disproof;   //disproof value, -1 = infinite
	int subTreeSize;  //indicate the number of PN nodes in the subtree
	short int subTreeDepth; //indicate the depth of the subtree
	short int PNnode;    //indicate the number of times that the node has been chosen by the PN
	
	//---- functions specific to  PNsearch ---
	void proved();    //update the variables after a node has been proved
	void disproved(); //update the variables after a node has been disproved
	
	//--------reimplement the virtual "static" functions of BaseTraversal ---------
	//note : they are not static because we need the derivation mecanism, only available on instances
	virtual void traversalNodeFinished(const storeId &id);
	virtual void initTraversalStatic();  //init the parameters of PN at the start of mainloop
	virtual bool traversalIteration();
	virtual Interface getParamDef();
	
	//---- static functions specific to PNsearch ----
	//note : these functions are used internally to PN-search and we can really use the keyword "static"
	static bool usePNheuristic;			//indicate if we use heuristics to initialize PN
	static storeId PNChosenId;
	
	static void updatePropagatePN(const storeId &id); //update the p/d values from the children, and propagate to the parent
	static void internalUpdatePropagatePN(const storeId &id); //internal function without the lock to allow recursive calls
	static void choosePNBranch();  //create the computing branch of the PN-search
	static storeId choosePNNextId(const storeId &id); //return the next id in the PN branch
	static void PN_initChosenId();
	static void provedNode(const storeId &id);
	static void disprovedNode(const storeId &id);
};

#endif
