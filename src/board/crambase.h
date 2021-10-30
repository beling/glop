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

#ifndef CRAMBASE_H
#define CRAMBASE_H

#include "splitboard.h"

//! Base Class for games like Cram
//! @ingroup BoardGroup
class CramBase : public SplitBoard {
  public:
	CramBase() {};
	
	bool possiblePlay(int i, int j, int direction);	//!< indicate if the play at i,j is possible
	bool play(int i, int j, int direction);			//!< play at the point i, j in the given direction
	bool isolated(int i0, int j0);					//!< Indicate if the given point is isolated or not
	void fillIsolatedCells();						//!< Fill isolated cells with Gray color
	
	string exportOptionalString();	//!<(optional, inherited from BaseGame) display the number of cells
	int countCells();				//!< count the number of empty cells
};

#endif
