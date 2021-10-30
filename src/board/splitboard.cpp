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

#include "splitboard.h"

#include <QString>

//return true if the position is composed of multiple boards
bool SplitBoard::isMultiComponents() {
	return (boardList.size() > (unsigned int) 1);
}

//return the list of components as individual positions
list<Line> SplitBoard::sumComponents() {
	list<Line> result;
	
	//we need to transform each string in a Line
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		//initialize the current board
		initCurrentBoard(index);
		result.push_back(toLine());
	}
	
	return result;
}

//---------------------- Algorithm to separate independent components------------------------------------------

//recursively color one connected component of the board
void SplitBoard::colorOneComponent(int i0, int j0, int colorNumber) {
	//color the current cell
	colors[i0*columns+j0]=colorNumber;
	cellsToColor--;
	
	//color recursively all the adjacent cells
	int direction;
	for(direction=0; direction<4 && cellsToColor>0; direction++) {
		int i=i0, j=j0;
		if(next(i, j, direction)) {
			//this is a valid direction
			//extend the connected component if the adjacent cell is not already colored
			if(colors[i*columns+j]<0) colorOneComponent(i, j, colorNumber);
		}
	}
}

//color the connected components of the board, with a different color for each component
//return the number of colors, which is also the number of components
int SplitBoard::colorComponents() {
	//if needed resize the vector used to store the colors associated to the cells
	if((int) colors.size() < rows*columns) colors.resize(rows*columns);
	cellsToColor = 0; //number of cells that need to be colored
	
	//color 0 means an occupied gray square, color -1 means any other square (considered empty)
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			if(getCell(i,j) != Gray) {
				colors[i*columns + j] = -1;
				cellsToColor++;
			} else {
				colors[i*columns + j] = 0;
			}
		}
	}
	
	int colorNumber = 0;
	
	//color the board until all cell are colored
	for(i=0; i<rows && cellsToColor>0; i++) {
		for(j=0; j<columns && cellsToColor>0; j++) {
			if(colors[i*columns + j]<0) {
				colorNumber++;
				colorOneComponent(i, j, colorNumber);
			}
		}
	}
	
	return colorNumber;
}

//Obtain the board corresponding to a given color (=one connected component)
//Cells different from the given color are filled with Gray
void SplitBoard::getOneComponent(int colorNumber) {
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			if(colors[i*columns + j]!=colorNumber) {
				setCell(i, j, Gray);
			}
		}
	}
}

//---------------------- Canonization -----------------------------------------

//by default, canonize with all height symmetries (vertical, horizontal, rotations)
int SplitBoard::symmetryForCano() {
	return BoardTable::EightSymmetries;
}

//canonization of a board list
//separate each board into independent components
void SplitBoard::canonize() {
	if(boardList.empty()) return;
	
	vector<string> result;
	
	//we need to separate into components each board of the board list
	int index;
	
	for(index = 0; index < (int) boardList.size(); index++) {
		//initialize the current board
		initCurrentBoard(index);
		if(!isCanonical()) {
			int colorNumber = colorComponents();
			if(colorNumber > 1) {
				saveBoard(); //getOneComponent changes the board, so we save it
				
				int i;
				for(i=1; i<=colorNumber; i++) {
					getOneComponent(i);
					reduceSize();
					symmetry(symmetryForCano());	//canonize the authorized symetries
					setCanonical(true);				//the board is now canonized
					result.push_back(toString());
					
					//caution : we need to go back to the initial board, in order to get correctly the next component
					backToSavedBoard();
				}
			} else
			if(colorNumber==1) {
				reduceSize();
				symmetry(symmetryForCano());	//canonize the authorized symetries
				setCanonical(true);				//the board is now canonized
				result.push_back(toString());
			}
		} else {
			result.push_back(toString());
		}
	}
	
	//update the boardList with the result list
	boardList = result;
	
	//apply canonization operations specific to a given game
	gameSpecificCanonization();
	
	//sort the list of boards
	sort(boardList.begin(), boardList.end());
}

//by default, there is no specific canonization
//a given game can reimplement this function in order to perform specific canonization operations
//these operations will be performed just after splitting the board in independent components
void SplitBoard::gameSpecificCanonization() {
	//nothing by default
}

//! Reduce the size of the board, by deleting all the surrounding filled rows or columns 
void SplitBoard::reduceSize() {
	//first step : find the smallest containing rectangle between (i0, j0) -> (i1,j1)
	//during the algorithm, (i0, j0) will only decrease, and (i1, j1) will only increase
	int i0=rows-1, j0=columns-1, i1=0, j1=0;
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			if(getCell(i, j) != Gray ) {
				//a non-gray cell gives new conditions on the smallest rectangle
				if(i<i0) i0 = i;
				if(j<j0) j0 = j;
				if(i>i1) i1 = i;
				if(j>j1) j1 = j;
			}
		}
	}
	
	//second step : reduce the size of the board to the rectangle i0, j0, i1, j1
	reduceBoardTo(i0, j0, i1, j1);
}

//---------------------- Order -----------------------------------------

//the following function is usefull to give priority to positions that are easy to separate in independent components 
// return the length of the shortest "remaining" part needed to complete a straigth cutting line
int SplitBoard::minStraightCuttingLine() {
	//look for the minimum horizontal line
	int minHor = columns;
	int lineEmpty;
	int i, j;
	for(i=0; i<rows; i++) {
		lineEmpty=0;
		for(j=0; j<columns; j++) {
			if(getCell(i, j) != Gray) lineEmpty++;
		}
		if(lineEmpty < minHor) minHor = lineEmpty;
	}
	
	//look for the minimum vertical line
	int minVer = rows;
	for(j=0; j<columns; j++) {
		lineEmpty=0;
		for(i=0; i<rows; i++) {
			if(getCell(i, j) != Gray) lineEmpty++;
		}
		if(lineEmpty < minVer) minVer = lineEmpty;
	}
	
	//return the minimum between horizontal and vertical candidates
	if(minHor < minVer) {
		return minHor;
	} else {
		return minVer;
	}
}

//return a "score" representing the proportion of gray cells in the center of the board
//this function can be used in combination with minStraightCuttingLine to cut the boards in their center line
int SplitBoard::surroundingCellsScore() {
	//give the priority to positions with a lot of used cells on the surrounding of the board
	int score=0;
	
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			if(getCell(i,j) == Gray) {
				//this is a used cell, it gives points
				//score is proportionnal to the distance to the center of the board
				score += (rows/2 - i) * (rows/2 - i) + (columns/2 - j) * (columns/2 - j);
			}
		}
	}
	
	return score;
}

//return the remaining length (number of cells not gray) of the central line
//the parameter allows to get the first and second vertical lines, or the first and second horizontal line
//there are two different possible vertical lines when the number of links is even 
int SplitBoard::centralLineLength(int v) {
	int length=0;
	if(v == 0 || v== 1) {
		//cut along one of the main vertical line
		int centralColumn = columns/2;
		if(centralColumn % 2 == 0) {
			if(v==0) {
				centralColumn++;
			} else {
				centralColumn--;
			}
		}
		
		int i;
		for(i=0; i<rows; i++) {
			if(getCell(i, centralColumn) != Gray) length++;
		}
	} else {
		//cut along one of the main horizontal line
		int centralRow = rows/2;
		if(centralRow % 2 == 0) {
			if(v==2) {
				centralRow++;
			} else {
				centralRow--;
			}
		}
		
		int j;
		for(j=0; j<columns; j++) {
			if(getCell(centralRow, j) != Gray) length++;
		}	
	}
	return length;
}

//Weight for Dots
//Focus on cutting moves in the central lines, then on central moves
//if we suppose that columns>=rows, there are :
//1 central line for a rectangle with an even number of coins in a row (2 central lines for a square)
//2 central lines for a rectangle with an odd number of coins in a row (4 central lines for a square)
int SplitBoard::weightForCentralMoves() {
	int length_A=0;
	int length_B=0;
	int length_C=0;
	int length_D=0;
	int length_tmp;
	int centralColumn = columns/2;
	int i,j;
	
	//compute the lengths of the central lines
	if(centralColumn%2==1) {	//even number of coins in a row
		for(i=0; i<rows; i++) {
			if(getCell(i, centralColumn) != Gray) length_A++;
			if(rows==columns){	//case of a square
				if(getCell(centralColumn, i) != Gray) length_B++;
			}
		}
	} else {			//odd number of coins in a row
		for(i=0; i<rows; i++) {
			if(getCell(i, centralColumn-1) != Gray) length_A++;
			if(getCell(i, centralColumn+1) != Gray) length_B++;
			if(rows==columns){	//case of a square
				if(getCell(centralColumn-1, i) != Gray) length_C++;
				if(getCell(centralColumn+1, i) != Gray) length_D++;
			}
		}
	}
	
	//sort the length_i so that length_A is minimal (ugly)
	if(length_D<length_C){
		length_tmp=length_C;
		length_C=length_D;
		length_D=length_tmp;
	}
	if(length_C<length_B){
		length_tmp=length_B;
		length_B=length_C;
		length_C=length_tmp;
	}
	if(length_B<length_A){
		length_tmp=length_A;
		length_A=length_B;
		length_B=length_tmp;
	}
	if(length_D<length_C){
		length_tmp=length_C;
		length_C=length_D;
		length_D=length_tmp;
	}
	if(length_C<length_B){
		length_tmp=length_B;
		length_B=length_C;
		length_C=length_tmp;
	}
	if(length_D<length_C){
		length_tmp=length_C;
		length_C=length_D;
		length_D=length_tmp;
	}
	
	//we prefer the minimal cutting line to have a small number of remaining edges
	int result=length_A; result*=8;
	//we prefer the second minimal cutting line to have a small number of remaining edges, etc.
	result+=length_B; result*=8;
	result+=length_C; result*=8;
	result+=length_D; result*=1000;
	
	//big weight on central edges, so that they will be cut ASAP
	int rtmp=0; int ctmp=0;
	for(i=0; i<rows; i++) {
	  if(i<=rows/2) rtmp++; else rtmp--;
		for(j=0; j<columns; j++) {
		  if(j<=columns/2) ctmp++; else ctmp--;
			if(getCell(i, j) != Gray) result+=(ctmp*rtmp);
	}}
	
	return result;
}
