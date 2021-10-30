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

#include "cram.h"

//there is a dependency to gameGlobal because of the use of win, loss, draw in isVictory

#include "../gameglobal.h"
#include <QString>

//register mecanism
Cram Cram::root(0);

int Cram::gameCategory() {
	return GameType::ImpartialGame;
}

//by default, init to the start position
Cram::Cram() {
	initEmptyBoard();
}

//initialize the board with the function Board::fromLine
Cram::Cram(const Line& board_ini) {
	fromLine(board_ini);
}

//initialize the board with the function Board::fromString
Cram::Cram(const string& board_ini) {
	fromString(board_ini);
}

//compute the set of children of the position
//we need to compute the children of each board if the position is composed of multiple boards
//TODO : many useless copies of string
void Cram::computeOptionsSet() {
	optionsSet().clear();
	if(boardList.empty()) return;
	
	//compute the children of each board of the list
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		saveBoard();
		string sav = boardList[index];
		
		int i, j;
		for(i=0; i<rows; i++) {
			for(j=0; j<columns; j++) {
				//forward horizontal direction
				if ( play(i, j, 0) ) {
					fillIsolatedCells(); //Fill isolated cells with gray color
					reduceSize(); //reduce size by deleting surrounding useless lines
					setCanonical(false); //after a move, the board is not canonical
					boardList[index] = toString();
					optionsSet().insert(exportToString());  //add this new children to the list of children
					backToSavedBoard();  //get back to the original board
				}
				
				//forward vertical direction
				if ( play(i, j, 2) ) {
					fillIsolatedCells(); //Fill isolated cells with gray color
					reduceSize(); //reduce size by deleting surrounding useless lines
					setCanonical(false); //after a move, the board is not canonical
					boardList[index] = toString();
					optionsSet().insert(exportToString());  //add this new children to the list of children
					backToSavedBoard();  //get back to the original board
				}
			}
		}
		
		boardList[index] = sav;
	}
}

//indicate (before the end of the game) if any player has won: not used for Cram
//note : the fact that perfectly symmetic positions are a loss has been tested
//but without notable effect, because perfectly symmetric positions are extremly rare
win_loss Cram::Victory() {
	return global::Nim_unknown;
}

//a position is terminal if no moves are possible
bool Cram::is_terminal_position() {
	if(boardList.empty()) return true;
	
	//compute the children of each board of the list
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		//saveBoard();
		
		int i, j;
		for(i=0; i<rows; i++) {
			for(j=0; j<columns; j++) {
				//forward horizontal direction
				//if ( play(i, j, 0) ) {
				if ( possiblePlay(i, j, 0) ) {
					//backToSavedBoard();  //get back to the original board
					return false;
				}
				
				//forward vertical direction
				//if ( play(i, j, 2) ) {
				if ( possiblePlay(i, j, 2) ) {
					//backToSavedBoard();  //get back to the original board
					return false;
				}
			}
		}
	}
	
	return true;
}

//initialize the parameters used to sort the positions
void Cram::computeOrderParameters(MoveOrdering& result) {
	//parameter used in nimber algorithm
	//result.param.push_back(0);
	
	//particular case that can arise in rctmisese : case of an empty board
	if(boardList.empty() || rows*columns==0) {
		result.param.push_back(0);
		result.param.push_back(0);
		result.param.push_back(0);
		result.param.push_back(0);
		result.param.push_back(0);
		return;
	}
	
	//priority to small boards
	result.param.push_back(rows*columns);
	
	//priority to positions with independent components
	result.param.push_back( - boardList.size());
	
	//priority to "symmetric" boards
	result.param.push_back( - symmetryDegree(2));
	
	//priority to boards with short remaining parts needed to complete a cutting line
	result.param.push_back(minStraightCuttingLine());
	
	//priority to positions with a lot of filled cells in the center of the board
	result.param.push_back(surroundingCellsScore());
}

//parameter interface
Interface Cram::getParamDef() {
	Board::distinguishPlayers = false;
	Interface result = Board::getParamDef();
	result.name="Cram";
	return result;
}

void Cram::setParam() {
	Board::distinguishPlayers = false;
	Board::setParam();
}

//name of the position for the parameters given in the interface
string Cram::getPositionName() {
	string name;	
	if(!Board::isClassicPosition) {
		//free start position
		name+=Board::givenPosition;
	} else {
		name+=(QString("%1x%2 board").arg(Board::boardRows).arg(Board::boardColumns)).toStdString();
	}
	return name;
}
