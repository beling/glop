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

#ifndef FULLTREENODE_H
#define FULLTREENODE_H

#include "../baseclass/basenode.h"
#include "rct.h"
#include "fulltreeinfo.h"

//! Node for computing the complete game tree of a game
//! @ingroup NodeGroup
class FullTreeNode : public BaseNode {
  public:
	NODE_REGISTER_MACRO(FullTreeNode)
	
	//override the functions of BaseNode
	NODE_FUNCTION_LIST
	virtual bool lockedComputation();
	static int dbPosFtiIndex;					//datastorage index for this algorithm
	static const int Rct_kind, CT_kind;
	
	//objects or functions specific to this algorithm
	Line positionA;				//main object
	Rct RctResult;				//computation result : Rct
	FullTreeInfo FTIResult;		//computation result : FullTreeInfo
	
	static bool computeFTI;
	static bool computeRct;
};

#endif
