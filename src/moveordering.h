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

#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include "global.h"

//Class to sort nodes
//all nodes and all games must use the following class to store the meaningful parameters for sort
//each game can define its own set of parameters and the following order will be used
//1.sort with the natural order on the list of integers
//2.if the two lists of integers are equal, sort with lexigrocaphical order on the string
class MoveOrdering {
  public :
	MoveOrdering();
	void clear();			//clear the internal obejcts as much as possible to free the memory used
	
	//parameters
	list<int> param;	//list of integer parameters
	string pos;			//string representation of the node
	win_loss checkValue;		//node known in a previous computation 0 =unknown, 1=known to be Win, 2 =known to be Loss
	
	bool withConjecture; //indicate whether we know a position conjectured to be similar
	Line conjecturedPos; //string representation of a position conjectured to be "similar"
	
	static bool ignoreCheckValue;	
	bool operator==(const MoveOrdering& a) const;
	bool operator<(const MoveOrdering& a) const;
};

#endif
