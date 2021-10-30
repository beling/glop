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

#ifndef SPLITBOARD_H
#define SPLITBOARD_H

#include "board.h"
#include "../baseclass/basegame.h"

//! Base Class for boards that can be splitted into independent components
//! @ingroup BoardGroup
class SplitBoard : public Board {
  public:
	SplitBoard() {};

	virtual bool isMultiComponents();		//!< indicate if there are multiple components
	virtual list<Line> sumComponents();  //!< independent components of the disjunctive sum
	
	void reduceSize();
	
	void colorOneComponent(int i0, int j0, int colorNumber); //!< Recursively color one connex components of the board
	int colorComponents(); //!< Color the connex components of the board
	void getOneComponent(int colorNumber);
	vector<int> colors;
	int cellsToColor; //number of cells that need to be colored
	
	virtual void canonize();
	virtual int symmetryForCano(); //!< used to indicate the possible symmetries
	
	virtual void gameSpecificCanonization(); //!< this function can be used by games to customize the canonization
	
	int minStraightCuttingLine();	//!< return the length of the shortest "remaining" straigth cutting line
	int surroundingCellsScore();	//!< return a "score" indicating how much gray cells are in the center of the board
	int centralLineLength(int v);
	int weightForCentralMoves();
};

#endif
