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

#include "moveordering.h"

//parameter used to ignore the check value in some algorithms
bool MoveOrdering::ignoreCheckValue=false;

MoveOrdering::MoveOrdering() {
	checkValue=0;
	withConjecture = false;
}

//reduce as possible the size of some internal objects
void MoveOrdering::clear() {
	param.clear();		//clear the list of parameters
	pos.clear();		//clear the string
	pos.reserve(0);		//reserve no memory for the string
}

bool MoveOrdering::operator==(const MoveOrdering& a) const {
	return ( (param == a.param) && (pos==a.pos) );
}

bool MoveOrdering::operator<(const MoveOrdering& a) const {
	if(!ignoreCheckValue && checkValue != a.checkValue) {
		//0=unknown, 1...100=known to be Win, 101...200=known to be Loss: priority to the biggest checkValues
		return (checkValue > a.checkValue);
	}
	
	if(param != a.param) {
		return (param<a.param);
	}
	
	return (pos<a.pos);
}
