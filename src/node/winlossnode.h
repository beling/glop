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

#ifndef WINLOSSNODE_H
#define WINLOSSNODE_H

#include "../baseclass/basenode.h"

//! Node for directly computing the win/loss of a game
//! @ingroup NodeGroup
class WinLossNode : public BaseNode {
  public:
	NODE_REGISTER_MACRO(WinLossNode)
	
	//override the functions of BaseNode
	NODE_FUNCTION_LIST
	virtual void prepareNextCycle(); //used to move the check database in the normal database during check cycles
	
	static int dbWinLossIndex, dbMisereIndex;			//datastorage index for this algorithm
	static const int NormalWL, MisereWL;				//computation kind for this algorithm
	
	//functions specific to this algorithm
	win_loss compute_children_Basic(list<BaseNode> &children);
	
	Line positionA;				//main object
	win_loss winLossResult;		//result type
	
	//used to debug the function Victory() for some games
	void compareToVictory();
	
	//parameters for this algorithm
	static bool misereVersion; //!< true = misere Version; false = normal version
	static bool debugVictory;  //!< true = compare Victory to the results obtained without it
};

#endif
