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

#ifndef RCTMISERENODE_H
#define RCTMISERENODE_H

#include "../baseclass/basenode.h"
#include "rct.h"

//class for the computation using Rct
class PosRct {
  public:
	PosRct() {nimCol = 0;};
	
	Line pos;				//position
	bool nimCol;			//boolean representing the nim-column 0 or 1
	multiset<Rct> RctSet;	//multiset of Rct
};

PosRct PosRctFromString(const string& stringA);
string PosRctToString(const PosRct& PosRctA);

//! Node for computing impartial games in the misere version with reduced canonical trees
//! @ingroup NodeGroup
class RctMisereNode : public BaseNode {
  public:
	NODE_REGISTER_MACRO(RctMisereNode)
	
	//override the functions of BaseNode
	NODE_FUNCTION_LIST
	virtual void prepareNextCycle(); //used to move the check database in the normal database during check cycles
	virtual string extraPositionPartTitle();	//title for the column with the part of the node that is not the position
	
	static int dbRctMisereIndex;						//datastorage for this algorithm
	static const int Misere_complex, RctInMisere, GrundyValue;		//computation kind for this algorithm
	
	//functions specific to this algorithm
	void apply_known_Rct();
	bool computeMisereChildren(list<BaseNode> &children);
	bool addNewChild(list<BaseNode> &children, const PosRct& posRcti);
	
	PosRct posRctA;				//main object
	win_loss winLossResult;		//result type
	
	static bool computeOutcome; //!< true = compute outcome; false = compute grundy-value
	static nimber currentGrundyValue; //used only when computing the grundy-value
};

#endif
