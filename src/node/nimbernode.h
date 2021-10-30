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

#ifndef NIMBERNODE_H
#define NIMBERNODE_H

#include "../baseclass/basenode.h"

//! Node for computing impartial games in the normal version with nimbers
//! @ingroup NodeGroup
class NimberNode : public BaseNode {
  public:
	NODE_REGISTER_MACRO(NimberNode)
	
	//override the functions of BaseNode
	NODE_FUNCTION_LIST
	virtual unsigned int traversalHeuristic();	//heuristic for the traversal of the tree
	virtual void prepareNextCycle();			//used to move the check database in the normal database during check cycles
	virtual string getPositionName();			//name of the start position set by the user in the interface
	virtual string extraPositionPartTitle();	//title for the column with the part of the node that is not the position
	
	static int dbNimberIndex;										//datastorage index for this algorithm
	static const int Nimber_kind, MultiComponents, Nimber_012; 	//computation kind for this algorithm
	
	//functions specific to this algorithm
	void apply_known_nimbers();	//!< use known nimbers to simplify a Node
	static nimber nimber_xor(list<nimber>& l_nimber);	//!< nimber resulting from a list of nimber
	win_loss compute_children_Nimber(list<BaseNode> &children);
	
	PositionString positionA;	//position part
	nimber nimberA;				//nimber part
	win_loss winLossResult;		//result type
	
	static const nimber Nim_max;
	
	//parameters for this algorithm
	//note : interface doesn't understand nimber type, so we use simply an int type here
	static int given_nimber;	//user-defined starting extended position (nimber part)
	static bool computeOutcome; //!< true = compute outcome; false = compute nimber
};

#endif
