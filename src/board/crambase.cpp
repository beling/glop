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

#include "crambase.h"

//there is a dependency to gameGlobal because of the use of win, loss, draw in isVictory

//#include "../gameglobal.h"
#include <QString>

//--------------------- Move Rules-------------------------

//Indicate whether a move is possible or not
//@return true if the play is valid
//this function doesn't modify the board, contrary to CramBase::play
bool CramBase::possiblePlay(int i, int j, int direction) { 
	if(getCell(i, j) != Empty) return false; //the current cell is not empty
	if( ! next(i, j, direction) ) return false; //invalid direction : we are out of the board
	if(getCell(i, j) != Empty) return false; //the adjacent cell is not empty
	return true;
}

//! Play at the point of coordinates i, j in the given direction.
//! @return true if the play is valid
//! Play is valid iff the two adjacent cells are empty.
bool CramBase::play(int i, int j, int direction) { 
	int i0 = i, j0 = j;
	
	if(getCell(i, j) != Empty) return false; //the current cell is not empty
	if( ! next(i, j, direction) ) return false; //invalid direction : we are out of the board
	if(getCell(i, j) != Empty) return false; //the adjacent cell is not empty
	
	//the two adjacent cells were empty, so the move is valid
	setCell(i0, j0, Gray);
	setCell(i, j, Gray);
	
	return true;
}

//--------------------- Board Reduction-------------------------

//! Indicate if the given point is isolated or not
bool CramBase::isolated(int i0, int j0) {
	int i, j, k;
	//0, 1, 2, 3 are the 4 vertical and horizontal directions
	for(k=0; k<4; k++) {
		i = i0;
		j=j0;
		if(next(i, j, k)) {
			//the adjacent cell in this direction is valid
			//If it is empty, then the given (i0, j0) point is not isolated
			if(getCell(i, j) == Empty) return false;
		}
	}
	
	return true;
}

//TODO : fillIsolatedCells could be optimized because only neighbour cells from the last move can become isolated
//! Fill isolated cells with Gray color
void CramBase::fillIsolatedCells() {
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			if(isolated(i, j)) {
				setCell(i, j, Gray);
			}
		}
	}
}

//--------------------- Display the number of cells in the interface -------------------------

string CramBase::exportOptionalString() {
	return int_to_string( countCells() );
}

//return the number of cells of the position
int CramBase::countCells() {
	int cellsNumber = 0;
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		int i, j;
		for(i=0; i<rows; i++) {
			for(j=0; j<columns; j++) {
				Vertex v = getCell(i, j);
				if ( v != Gray) {
					cellsNumber ++;
				}
			}
		}
	}
	
	return cellsNumber;
}
