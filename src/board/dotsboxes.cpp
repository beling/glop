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

#include "dotsboxes.h"

//there is a dependency to gameGlobal because of the use of win, loss, draw in isVictory

#include "../gameglobal.h"
#include "../stringconverter.h"
#include <QString>

//register mecanism
DotsAndBoxes DotsAndBoxes::root(0);

int DotsAndBoxes::dotsRows=3;
int DotsAndBoxes::dotsColumns=3;
string DotsAndBoxes::dotsGivenPosition;

bool DotsAndBoxes::isAmerican=true;
bool DotsAndBoxes::isSwedish=false;
bool DotsAndBoxes::isIcelandic=false;
bool DotsAndBoxes::useUltraWeak=false;
bool DotsAndBoxes::useChainOpening=false;

int DotsAndBoxes::storeLowCondition=0;
int DotsAndBoxes::storeUpCondition=0;

int DotsAndBoxes::gameCategory() {
	return GameType::ScoreGame;
}

//by default, init to the start position
DotsAndBoxes::DotsAndBoxes() {
	scoreVariation = 0;
	initDotsBoard();
}

//initialize the board with the function Board::fromLine
DotsAndBoxes::DotsAndBoxes(const Line& board_ini) {
	scoreVariation = 0;
	fromLine(board_ini);
}

//initialize the board with the function Board::fromString
DotsAndBoxes::DotsAndBoxes(const string& board_ini) {
	scoreVariation = 0;
	fromString(board_ini);
}

//init to an empty board of Dots and Boxes with black to play
void DotsAndBoxes::initDotsBoard() {
	//init the size of Board class from the number of dots
	Board::boardRows = 2*dotsRows-1;
	Board::boardColumns = 2*dotsColumns-1;
	
	//empty Dots and Boxes board of size Board::boardRows * Board::boardColumns
	//note : it contains only one board and the index is placed on this board
	initEmptyBoard();
	
	//"meaningless" cells in Gray (odd row, odd column)
	//between the coins : a link ("L") (odd row, even column) or (even row, odd column)
	//coins (even row, even column) on the boundary of the board with a single Arrow (number "1")
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			setCell(i, j, Coin);	//by default, a coin
			
			if(i%2==1 && j%2==1) setCell(i, j, Gray);
			if( (i%2==0 && j%2==1) || (i%2==1 && j%2==0) ) setCell(i, j, Link);
			
			if(i==0 || i== rows-1 || j==0 || j==columns-1) {
				if(isAmerican || (isIcelandic && i!=rows-1 && j!=0) ) {
					if(i%2==0 && j%2==0) setCell(i, j, SingleArrow);
				}
			}
		}
	}
	
	//coins at the corners with a DoubleArrow (number "2")
	if(isAmerican) {
		setCell(0, 0, DoubleArrow);
		setCell(0, columns-1, DoubleArrow);
		setCell(rows-1, 0, DoubleArrow);
		setCell(rows-1, columns-1, DoubleArrow);
	} else 
	if(isIcelandic) {
		setCell(0, 0, SingleArrow);
		setCell(0, columns-1, DoubleArrow);
		setCell(rows-1, columns-1, SingleArrow);
	}
}

//parameter interface
Interface DotsAndBoxes::getParamDef() {
	Interface result;
	result.name="Dots and Boxes";
	result.addLabel("Box rows :", /*pos*/ 0, 0);
	result.addSpinBox(2, 30, 3, /*pos*/ 0, 1);
	result.link(dotsRows, "BoxRows");
	
	result.addLabel("Box columns :", /*pos*/ 0, 2);
	result.addSpinBox(2, 30, 3, /*pos*/ 0, 3);
	result.link(dotsColumns, "BoxColumns");
	
	result.addRadioButton("American start position", true, /*pos*/ 1, 0);
	result.link(isAmerican, "isAmerican");
	result.addRadioButton("Swedish start position", false, /*pos*/ 1, 1);
	result.link(isSwedish, "isSwedish");
	result.addRadioButton("Icelandic start position", false, /*pos*/ 1, 2);
	result.link(isIcelandic, "isIcelandic");
	result.addRadioButton("Given start position", false, /*pos*/ 2, 0);
	
	Board::distinguishPlayers = false;
	DotsAndBoxes b; //create an empty board of dots and boxes
	result.addLineEdit(b.toString(), /*pos*/ 2, 1, 1, 4);
	result.link(dotsGivenPosition, "GivenPosition");
	
	result.addCheckButton("Use ultra-weak", true, /*pos*/ 3, 0);
	result.link(useUltraWeak, "ultraWeak");
	result.addCheckButton("Use chain opening", true, /*pos*/ 3, 1);
	result.link(useChainOpening, "chainOpening");
	
	#ifdef UNSUPPORTED_FEATURE
	result.addLabel("Store low condition (<=edges) :", /*pos*/ 4, 0);
	result.addSpinBox(0, 100, 16, /*pos*/ 4, 1);
	result.link(storeLowCondition, "StoreLowCond");
	result.addLabel("Store up condition (>edges) :", /*pos*/ 4, 2);
	result.addSpinBox(0, 100, 35, /*pos*/ 4, 3);
	result.link(storeUpCondition, "StoreUpCond");
	#endif
	
	return result;
}

//name of the position for the parameters given in the interface
string DotsAndBoxes::getPositionName() {
	string message;
	
	if(!isAmerican && !isSwedish && !isIcelandic) {
		//free start position
		message+=dotsGivenPosition;
	} else {
		message+=(QString("%1x%2 boxes").arg(dotsRows).arg(dotsColumns)).toStdString();
	}
	
	return message;
}

void DotsAndBoxes::setParam() {
	//initialize the static variable of Board class
	Board::distinguishPlayers = false;
	
	//start position
	Parameter::usePositionString = true;
	if(isAmerican || isSwedish || isIcelandic) {
		DotsAndBoxes b;  //empty board of dots and boxes
		Parameter::given_positionString=b.toString();
	} else {
		//given start position
		Parameter::given_positionString=dotsGivenPosition;
	}
}

string DotsAndBoxes::exportOptionalString() {
	return int_to_string( countEdges() );
}

//title for the column displaying the optional string
string DotsAndBoxes::optionalStringTitle() {	
	return string("Edges");
}

//indicate if the position should be stored permanently or not
//return true if there are less edges than the low conditionw or strictly more than the up condition
bool DotsAndBoxes::isPermanentPosition() {
	int edgesNumber = countEdges();
	return (edgesNumber<=storeLowCondition || edgesNumber>storeUpCondition );
}

//return the number of edges of the position
int DotsAndBoxes::countEdges() {
	//number of remaining edges
	int edgesNumber = 0;
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		int i, j;
		for(i=0; i<rows; i++) {
			for(j=0; j<columns; j++) {
				Vertex v = getCell(i, j);
				if ( v == DoubleArrow) {
					edgesNumber += 2;
				} else
				if( v == SingleArrow || v == Link ) {
					edgesNumber++;
				}
			}
		}
	}
	
	return edgesNumber;
}

//indicate the number of remaining boxes for this position
ScoreRange DotsAndBoxes::availableScore(bool maxScoreOnly) {
	ScoreRange scoreLimit;
	
	//------------------------Number of available boxes---------------
	int remainingBoxes = 0;
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		int i, j;
		//note : coins are only in (even, even) board positions
		for(i=0; i<rows; i+=2) {
			for(j=0; j<columns; j+=2) {
				Vertex v=getCell(i, j);
				if ( v == DoubleArrow || v == SingleArrow || v == Coin ) remainingBoxes++;
			}
		}
	}
	scoreLimit.decreaseMaxTo(remainingBoxes);
	
	//------------------------UltraWeak theorem-------------------------
	if(useUltraWeak && !maxScoreOnly) {
		//number of captured boxes if the ultra-weak theorem applies
		int capturedBoxes = ultraWeakTheorem();
		if(capturedBoxes != -1) {
			int minScore = (remainingBoxes + capturedBoxes + 1) / 2;
			scoreLimit.increaseMinTo(minScore);
		}
	}
	
	return scoreLimit;
}

// used to indicate the score modification
//empty if there is no modification
string DotsAndBoxes::gameSpecificString() {
	string result;
	if(scoreVariation!=0) {
		result += 'J';
		StringConverter conv;
		conv << scoreVariation;
		result += conv.getString();
	}
	return result;
}

//compute the set of children of the position
//we need to compute the children of each board if the position is composed of multiple boards
//Linear chains (with at least one internal coin)  : all the hildren are equivalent, the first one is sufficient
void DotsAndBoxes::computeOptionsSet() {
	optionsSet().clear();
	if(boardList.empty()) return;
	bool capturableExist = whiteBlackTokenTheorem();
	
	//compute the children of each board of the list
	bool movePossible = false;
	int index;
	unsigned int boardNumber = boardList.size();
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		saveBoard();
		string sav = boardList[index];
		
		int i, j;
		for(i=0; i<rows; i++) {
			for(j=0; j<columns; j++) {
				Vertex v = getCell(i, j);
				if ( v == DoubleArrow || v == SingleArrow || v == Link ) {
					movePossible = true;
					if(v == DoubleArrow) {
						setCell(i, j, SingleArrow);
					} else
					if(v == SingleArrow) {
						setCell(i, j, Coin);
					} else {
						//v == Link
						setCell(i, j, Gray);
					}
					
					if(useChainOpening) chainOpeningEquivalences(i, j);
					setCanonical(false); //after a move, the board is not canonical
					boardList[index] = toString();
					addBoardToOptionsSet();
					
					if(boardList.size() > boardNumber) {
						//it is possible that a board has been added in chainOpeningEquivalences 
						boardList.pop_back();
					}
					backToSavedBoard();  //get back to the original board
					scoreVariation = 0;  //initialize back the scoreVariation
				}
			}
		}
		
		boardList[index] = sav;
	}
	
	//special case : capturable move is possible but no move is possible after (end of the game)
	//the board with all the boxes captured is a move in itself
	if(capturableExist && !movePossible) addBoardToOptionsSet();
}

//add the curent Board List to the set of options
void DotsAndBoxes::addBoardToOptionsSet() {
	optionsSet().insert(exportToString());  //for scorenode, no distinction between black and white players
}

//quick function that returns an estimation of the number of children
int DotsAndBoxes::estimateChildrenNumber() {
	int childrenNumber = 0;
	//compute the children of each board of the list
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		
		int i, j;
		for(i=0; i<rows; i++) {
			for(j=0; j<columns; j++) {
				Vertex v = getCell(i, j);
				if ( v == DoubleArrow || v == SingleArrow || v == Link ) {
					childrenNumber++;
				}
			}
		}
	}
	return childrenNumber;
}

//indicate (before the end of the game) if any player has won
win_loss DotsAndBoxes::Victory() {
	return global::Nim_unknown;
	
}

//this function is not used. Victory() or availableScore() are always used instead
bool DotsAndBoxes::is_terminal_position() {
	//always return false in order so that Victory() is used
	return false;
}

//initialize the parameters used to sort the positions
//note : the parameters in front have a highest priority than parameters at the back
void DotsAndBoxes::computeOrderParameters(MoveOrdering& result) {
	//----------avoid to open a chain (i.e avoid almost completed box)-----
	int almostCompletedBox=0;
	//int arrowsNumber=0;
	int index;
	//int minLineMostDifficultBoard=0;
	//int surroundingCells = 0;
	//int degree2=0;
	//int degree3=0;
	//int degree4=0;
	int biggestBoard=0;
	int centralMoves=0;
	int totalBoardsSize=0;
	/*int biggestBoardCentralLength=0;
	int biggestBoardCentralLengthB=0;
	int biggestBoardCentralLengthC=0;
	int biggestBoardCentralLengthD=0;*/
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		/*
		int minLine = minStraightCuttingLine();
		if(minLine > minLineMostDifficultBoard) {
			minLineMostDifficultBoard = minLine;
			surroundingCells = surroundingCellsScore();
		}*/
		totalBoardsSize+=rows*columns;
		if(rows*columns>biggestBoard) {
			biggestBoard=rows*columns;
			centralMoves=weightForCentralMoves();
			/*biggestBoardCentralLength  = centralLineLength(0);
			biggestBoardCentralLengthB = centralLineLength(1);
			biggestBoardCentralLengthC = centralLineLength(2);
			biggestBoardCentralLengthD = centralLineLength(3);*/
			
		}
		
		int i, j;
		//note : coins only at (even, even) board positions
		for(i=0; i<rows; i+=2) {
			for(j=0; j<columns; j+=2) {
				Vertex v = getCell(i, j);
				if(v == Coin || v == SingleArrow) {
					//if the box is almost completed, increment the counter
					if(coinDegree(i, j, /*countArrows*/ true)==1) almostCompletedBox++;
				}
				/*
				if(v != Gray && v!=Link) {
					int degree = coinDegree(i, j, false); //conoutArrows = false
					if(degree==2) {
						degree2++;
					} else
					if(degree==3) {
						degree3++;
					} else
					if(degree==4) {
						degree4++;
					}
				}
				*/
				
				/*if( v == DoubleArrow ) {
					arrowsNumber += 2;
				} else
				if( v == SingleArrow ) {
					arrowsNumber++;
				}*/
			}
		}
	}
	
	//priority to positions with a small number of almost completed box
	result.param.push_back(almostCompletedBox);
	
	//small number of degrees 3 and 4, big number of degree 2
	//result.param.push_back(degree4);
	//result.param.push_back(degree3);
	//result.param.push_back(-degree2);
	
	//priority to positions with a lot of independent boards
	//result.param.push_back(-boardList.size());
	result.param.push_back(biggestBoard);
	
	//priority to cutting moves that give small boards
	result.param.push_back(totalBoardsSize);
	
	//priority to cutting moves in the central lines, then to central moves
	result.param.push_back(centralMoves);
	
	//priority to boards with short remaining parts needed to complete a cutting line
	//we use the minimum value of the "most difficult board" (max of min)
	//result.param.push_back(minLineMostDifficultBoard);
	/*result.param.push_back(biggestBoardCentralLength);
	result.param.push_back(biggestBoardCentralLengthB);
	result.param.push_back(biggestBoardCentralLengthC);
	result.param.push_back(biggestBoardCentralLengthD);*/
	
	//priority to boards with a lof of gray cell in the center of the board
	//result.param.push_back(surroundingCells);
	
	//priority to positions with a lot of arrows : bad effect on 3x5ice8
	//result.param.push_back(-arrowsNumber);
	
	//priority to positions with a small number of children : no clear effect
	//computeOptionsSet();
	//result.param.push_back(optionsSet().size());
	
	//result.param.push_back(estimateChildrenNumber());
	
	//priority to "symmetric" boards : no clear effect
	//result.param.push_back( - symmetryDegree(2));
}

//------------------------- White-Black Token Theorem---------------------
//when some coins can be taken, some moves are dominated by others
//there are essentially only two kind of moves :
//kind 1 : take all the coins and play a move that don't take any coin
//kind 2a : take all the coins except two coins and make this pair of coins independent
//kind 2b : if there is no move 2a, take all the coins except four.

//step 1 : if there is a move 2a or 2b, add it to the list of children
//step 2 : color all the boxes with the current player color
bool DotsAndBoxes::whiteBlackTokenTheorem() {
	//color the part of the position that is capturable
	int capturableBoxesNumber = colorCapturableComponent();
	if(capturableBoxesNumber == 0) return false;
	
	vector<string> boardListCompleteSav = boardList;
	
	//search the existence of two capturable coins linked to an arrow or a non-capturable coin
	if(searchCoinPair()) {
		//there exists a "2a move".
		attributeColoredComponent();
		if(useChainOpening) {
			//we add a pair of coins, but it will necessary be captured by the opponent
			//so we simply change the score
			scoreVariation = 2;
		} else {
			boardList.push_back(coinPairString()); //add a pair of coins to the the list of boards
		}
		addBoardToOptionsSet();
		scoreVariation = 0;
		boardList = boardListCompleteSav;
	} else 
	if(searchDoublePair()) { //search the existence of four capturable coins in a straight line
		//there exists a "2b move".
		attributeColoredComponent();
		if(useChainOpening) {
			//we add two pairs of coins, but it will necessary be captured by the opponent
			//so we simply change the score
			scoreVariation = 4;
		} else {
			boardList.push_back(coinPairString()); //add a pair of coins to the the list of boards
			boardList.push_back(coinPairString()); //add another pair of coins
		}
		addBoardToOptionsSet();
		scoreVariation = 0;
		boardList = boardListCompleteSav;
	}
	
	//color all the boxes with the player color and remove the colored links
	attributeColoredComponent();
	return true;
}

//color the boxes that the next player is sure to capture
//(i0, j0) is the cell where the current player has played
bool DotsAndBoxes::chainOpeningEquivalences(int i0, int j0) {
	int capturableBoxesNumber = 0;
	
	//check first the cell (and the surrouding ones) where the current player moved
	//by default i0 and j0 equals -1, for compatibility with dotsboxes_test olf test functions
	if(i0!=-1 && j0!=-1) {
		Vertex v = getCell(i0, j0);
		if(v==SingleArrow || v==Coin) {
			if(colorCapturableCoin(i0, j0)) capturableBoxesNumber++;
		} else {
			//it means the current cell is a gray one, and was a link before
			//we need to check the 4 surrounding coins
			int i1, j1, k;
			for(k=0; k<4; k++) {
				i1 = i0;
				j1 = j0;
				if(next(i1, j1, k)) { //move (i1, j1) in the direction of k
					//the adjacent cell in this direction is valid
					if(colorCapturableCoin(i1, j1)) capturableBoxesNumber++;
				}
			}
		}
		if(capturableBoxesNumber == 0) return false;
	}
	
	//color the remaining part of the position that is capturable
	capturableBoxesNumber += colorCapturableComponent_currentBoard();
	if(capturableBoxesNumber == 0) return false;
	
	//search the existence of two capturable coins linked to an arrow or a non-capturable coin
	if(searchCoinPair_currentBoard()) {
		//there exists a "2a move".
		attributeColoredComponent_currentBoard();
		scoreVariation = capturableBoxesNumber - 2;
		boardList.push_back(linkedCoinPairString());
	} else 
	if(searchDoublePair_currentBoard()) { //search the existence of four capturable coins in a straight line
		//there exists a "2b move".
		attributeColoredComponent_currentBoard();
		scoreVariation = capturableBoxesNumber - 4;
		boardList.push_back(fourCoinsString());
	} else {
		//we only capture all the possible boxes
		scoreVariation = capturableBoxesNumber;
		attributeColoredComponent_currentBoard();
	}
	return true;
}

//Implementation of the ultra-weak theorem, that gives a bound on the possible score in some cases
//return -1 if the ultra-weak theorem tells nothing about the current position
//if the ultra-weak theorem applies, return the number of captured boxes
int DotsAndBoxes::ultraWeakTheorem() {
	//color the part of the position that is capturable
	int capturableBoxesNumber = colorCapturableComponent();
	
	//ultra-weak theorem does not apply if at least two boxes are not capturable
	if(capturableBoxesNumber < 2) return -1;
	
	if(searchCoinPair()) {
		//there exists two capturable coins linked to an arrow or a non-capturable coin
		//ultra-weak theorem applies
		return (capturableBoxesNumber-2);
	} else 
	if(searchDoublePair()) {
		//there exists four capturable coins in a straight line
		//ultra-weak theorem applies
		return (capturableBoxesNumber-4);
	} else {
		//there is no pair or double-pair of coins
		//ultra-weak theorem does not apply
		return -1;
	}
}

//return the string representing a pair of independent coins
string DotsAndBoxes::coinPairString() {
	string result;
	result += Coin;
	result += Link;
	result += Coin;
	result += SizeMark;
	result += EndCanonicalBoard;
	return result;
}

string DotsAndBoxes::linkedCoinPairString() {
	string result;
	result += SingleArrow;
	result += Link;
	result += Coin;
	result += SizeMark;
	result += EndCanonicalBoard;
	return result;
}

string DotsAndBoxes::fourCoinsString() {
	string result;
	result += Coin;
	result += Link;
	result += Coin;
	result += Link;
	result += Coin;
	result += Link;
	result += Coin;
	result += SizeMark;
	result += EndCanonicalBoard;
	return result;
}

//color the capturable boxes
//return the number of capturable boxes
//NOTE : in this function, the boards are supposed to be canonic
//i.e capturable components can only be of the form linkedCoinPairString or fourCoinsString
//this function is called from ultraWeakTheorem and whiteBlackTokenTheorem (the boards are canonic)
int DotsAndBoxes::colorCapturableComponent() {
	//color the capturable coins of each board of the list
	int capturableNumber = 0;
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		if(rows==1 && (columns==3 || columns==7)) {
			capturableNumber += colorCapturableComponent_currentBoard();
			if(capturableNumber!=0) {
				setCanonical(false);			//we have modified the board, which is not canonical anymore
				boardList[index] = toString();
			}
		}
	}
	return capturableNumber;
}

int DotsAndBoxes::colorCapturableComponent_currentBoard() {
	int capturableNumber = 0;
	bool repeat = true;
	int i, j;
	while(repeat) {
		repeat = false;
		//note : coins only at (even, even) board positions
		for(i=0; i<rows; i+=2) {
			for(j=0; j<columns; j+=2) {
				//if at least one coin is capturable, we will need to repeat the coloring
				if(colorCapturableCoin(i, j)) {
					repeat = true;
					capturableNumber++;
				}
			}
		}
	}
	return capturableNumber;
}

//color a coin and its links if it is capturable
//return true if the coin is capturable 
bool DotsAndBoxes::colorCapturableCoin(int i, int j) {
	Vertex v = getCell(i, j);
	
	//only an empty cell (internal coin) or a single arrow (coin with one arrow) can be capturable
	if ( v != Coin && v!= SingleArrow) return false;
	
	//it is capturable only if this coin has only one link or one arrow left
	//we check the surrounding cells in horizontal and vertical directions
	//note : we don't count already colored linked because they have already been "removed"
	int linkCount=0;
	if(v == SingleArrow) linkCount++;
	int i2, j2, iLink=0, jLink=0, k;
	for(k=0; k<4; k++) {
		i2 = i;
		j2 = j;
		if(next(i2, j2, k)) { //move (i2, j2) in the direction of k
			//the adjacent cell in this direction is valid
			//If it is a link, increment the number of counted links
			if(getCell(i2, j2) == Link) {
				linkCount++;
				if(linkCount>1) return false;	//it is not capturable, return immediately
				iLink = i2;
				jLink = j2;
			}
		}
	}
	
	//if the box is capturable, color it and also its link
	if(linkCount==1) {
		if(v == SingleArrow) {
			setCell(i, j, ColoredSingleArrow);
		} else {
			//case of an internal coin
			setCell(i, j, ColoredCoin);
			setCell(iLink, jLink, ColoredLink);
		}
		return true;
	}
	
	//it is possible that a coin was captured by a previous move (i.e a link that captures two boxes in one move)
	//we take into account this case here, be capturing coins without any link
	if(v != SingleArrow && linkCount==0) {
		//internal coin already captured
		setCell(i, j, ColoredCoin);
		return true;
	}
	
	return false;
}

//attribute the colored component to the current player
void DotsAndBoxes::attributeColoredComponent() {
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		attributeColoredComponent_currentBoard();
		boardList[index] = toString();
	}
}

void DotsAndBoxes::attributeColoredComponent_currentBoard() {
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			Vertex v = getCell(i, j);
			if(v == ColoredSingleArrow || v == ColoredCoin || v == ColoredLink) {
				setCanonical(false);	//we modify the board, which is not more canonical
				setCell(i, j, Gray);
			}
		}
	}
}

//search a colored coin pair linked to an arrow or an uncolored coin
//return true if a pair was found
bool DotsAndBoxes::searchCoinPair() {
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		if(searchCoinPair_currentBoard()) return true;
	}
	
	return false;
}

bool DotsAndBoxes::searchCoinPair_currentBoard() {
	int i, j;
	//note : coins only at (even, even) board positions
	for(i=0; i<rows; i+=2) {
		for(j=0; j<columns; j+=2) {
			if(isCoinPair(i, j)) return true;
		}
	}
	return false;
}

//search a double pair of colored coin pair linked only to colored coins
//return true if a double pair was found
//NOTE : the structure of this function is very similar to searchCoinPair
bool DotsAndBoxes::searchDoublePair() {
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		if(searchDoublePair_currentBoard()) return true;
	}
	
	return false;
}

bool DotsAndBoxes::searchDoublePair_currentBoard() {
	int i, j;
	//note : coins only at (even, even) board positions
	for(i=0; i<rows; i+=2) {
		for(j=0; j<columns; j+=2) {
			if(isDoublePair(i, j)) return true;
		}
	}
	return false;
}

//search from the current cell a pair of coins linked to an arrow or a non-capturable coin
bool DotsAndBoxes::isCoinPair(int i, int j) {
	Vertex v = getCell(i, j);
	if(v==ColoredSingleArrow) {
		//search a colored link followed by a colored coin
		return searchColoredLinked(i, j, /*linkedCoinWithColor*/ true, /*length*/ 1);
	} else
	if(v==ColoredCoin) {
		//--------step 1 : search a colored link followed by someting uncolored-----
		if(!searchColoredLinked(i, j, /*linkedCoinWithColor*/ false, /*length*/ 1)) return false;
		
		//-------step 2 : search a colored link followed by a colored coin -------
		return searchColoredLinked(i, j, /*linkedCoinWithColor*/ true, /*length*/ 1);
	}
	
	return false;
}

//search a colored link from the given cell, and check the color of the linked coin
//return true if we have found the pattern
//forbiddenDirection indicates a direction in which it is forbidden to search a colored link
//    it is used to avoid going back to a previous coin when looking for a double pair
//    by default, forbiddenDirection = -1 and means that all directions are possible
//length indicates the length of the pattern that we search
bool DotsAndBoxes::searchColoredLinked(int i, int j, bool linkedCoinWithColor, int length, int forbiddenDirection) {
	//search a colored link
	int i2, j2, k;
	for(k=0; k<4; k++) {
		if(k != forbiddenDirection) {
			i2 = i;
			j2 = j;
			if(next(i2, j2, k)) { //move (i2, j2) in the direction of k
				//the adjacent cell in this direction is valid
				//If it is a colored link, look in the same direction and compare the color of the linked coin with the parameter
				if(getCell(i2, j2) == ColoredLink) {
					if(next(i2, j2, k)) { //note : it should always be true (move in the direction of the link always possible)
						Vertex linkedCoin = getCell(i2, j2);
						bool isColored = (linkedCoin == ColoredCoin || linkedCoin == ColoredSingleArrow);
						if(isColored == linkedCoinWithColor) {
							if(length == 1) {
								//this is the end of the pattern
								return true;
							} else {
								//we need to search the next part of the pattern
								//the reverse of the current direction is forbidden ( 0 <->1 ; 2<->3 )
								int nextForbiddenDirection=0;
								switch(k) {
									case 0: nextForbiddenDirection = 1; break;
									case 1: nextForbiddenDirection = 0; break;
									case 2: nextForbiddenDirection = 3; break;
									case 3: default: nextForbiddenDirection = 2;
								}
								
								if(searchColoredLinked(i2, j2, linkedCoinWithColor, length-1, nextForbiddenDirection)) return true;
							}
						}
					}
				}
			}
		}
	}
	
	return false;
}

//search from the current cell a pair of coins linked to an arrow or a non-capturable coin
bool DotsAndBoxes::isDoublePair(int i, int j) {
	if(getCell(i, j)!=ColoredCoin) return false;
	return searchColoredLinked(i, j, /*linkedCoinWithColor*/ true, /*length*/ 3);
}

//----------------------------------------Linear Chains---------------------------------------------
//a linear chain is a string of coins of degree two with an arrow at each end
//the length of the chain is the number of internal coins (those not linked to arrows)
//we consider only chains with at least one internal coin

//indicate if the current board is a linear chain
//return -1 if it is not a linear chain, otherwise return the length of the chain
//note : we suppose here that the independent component of the boards have already been separated
//in that case, a chain can be characterized only with the coins degree
//we need only two arrows and all coins of degree exactly 2
int DotsAndBoxes::isLinearChain() {
	int length = 0;
	int arrowsNumber=0;
	
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			Vertex v = getCell(i, j);
			if(v == DoubleArrow) return -1; //no double arrows possible
			if(v == SingleArrow) {
				arrowsNumber++;
				if(arrowsNumber > 2) return -1; //no more than 2 arrows
			}
			
			if(v != Link && v!= Gray) {
				//this is a coin
				length++;
				if(coinDegree(i, j, /*countArrows*/ true) != 2) return -1; //all coins must have degree 2
			}
		}
	}
	
	if(length > 2 ) {
		return length - 2;
	} else {
		return -1;
	}
}

//return the degree of a coin
int DotsAndBoxes::coinDegree(int i, int j, bool countArrows) {
	int linkCount=0;
	if(countArrows) {
		Vertex v = getCell(i, j);
		if(v == SingleArrow) {
			linkCount = 1;
		} else
		if(v == DoubleArrow) {
			linkCount = 2;
		}
	}
	
	int i2, j2, k;
	for(k=0; k<4; k++) {
		i2 = i;
		j2 = j;
		if(next(i2, j2, k)) { //move (i2, j2) in the direction of k
			//the adjacent cell in this direction is valid
			//If it is a link, increment the number of counted links
			if(getCell(i2, j2) == Link) linkCount++;
		}
	}
	
	return linkCount;
}

//a caterpillar is a string of coins all of degree 2, with 2 heads of degree 1
//arrows are not counted in the degree
string DotsAndBoxes::canonizeCaterpillar() {
	string canonicalString;
	int head=0, headi=0, headj=0;
	
	int i, j;
	//note : coins only at (even, even) board positions
	for(i=0; i<rows; i+=2) {
		for(j=0; j<columns; j+=2) {
			Vertex v = getCell(i, j);
			if(v != Link && v!= Gray) {
				int degree = coinDegree(i, j, /*countArrows*/ false);
				
				//all coins must have degree 1 or 2
				if(degree > 2 || degree < 1) return canonicalString;
				
				//the caterpillar can have only two heads
				if(degree==1) {
					head++;
					headi=i;
					headj=j;
				}
				if(head>2) return canonicalString;
			}
		}
	}
	
	//there must be exactly two heads
	if(head != 2) return canonicalString;
	
	//the current board is a caterpillar
	//create its linear canonical representation
	bool end=false;
	//start from the head
	i=headi;
	j=headj;
	
	while(!end) {
		canonicalString += getCell(i,j);
		setCell(i, j, Gray);	//gray the current cell
		
		//find the next cell
		int i2, j2, k;
		bool found=false;
		for(k=0; k<4 && !found; k++) {
			i2 = i;
			j2 = j;
			if(next(i2, j2, k)) { //move (i2, j2) in the direction of k
				//the adjacent cell in this direction is valid
				if(getCell(i2, j2) != Gray) {
					//we have found the next cell of the caterpillar
					i=i2;
					j=j2;
					found = true;
				}
			}
		}
		
		//if there was no next cell, this is the end of the caterpillar
		if(!found) end = true;
	}
	
	canonicalString += Board::SizeMark;
	canonicalString += Board::EndBoard;
	return canonicalString;
}

//canonize the orientation of the right angles
bool DotsAndBoxes::canonizeRightAngles() {
	bool canonizationOccured = false;
	int i, j;
	//note : coins only at (even, even) board positions
	for(i=0; i<rows; i+=2) {
		for(j=0; j<columns; j+=2) {
			if(canonizeRightAngle_oneBlock(i, j)) {
				canonizationOccured = true;
			}
		}
	}
	return canonizationOccured;
}

//Canonize the right angle contained in the 3x3-block beginning at (i,j) 
//Return true if some right angle have been canonized
//coin (i,j)    ---- link (i, j+1) ---- coin (i, j+2)
//      |                  |                  |
//link (i+1, j) ----    gray cell  ---- link (i+1,j+2)
//      |                  |                   |  
//coin (i+2, j) ---- link (i+2, j+1) -- coin (i+2, j+2)
bool DotsAndBoxes::canonizeRightAngle_oneBlock(int i, int j) {
	//we need a valid 3x3 block from the cell i,j to the cell i+2, j+2
	if(! ((i+2 < rows) && (j+2 < columns)) ) return false;
	
	//we need :
	// - exactly one gray cell in one of the angle
	// - three coins in the other angles
	// - links from the coin at the angle opposite to the gray coin
	// - a coin of degree 2 (without counting arrows) at the angle opposite to the gray coin 
	int grayCoins = 0;
	Vertex v1 = getCell(i, j);
	if(v1==Gray) {
		grayCoins++;
		if(getCell(i+1, j+2) != Link || getCell(i+2, j+1) != Link) return false; //check the links from the opposite coin
		if(coinDegree(i+2, j+2,  /*countArrows*/ false) != 2) return false;	//check the degree of the opposite coin
	} else {
		if(v1 != Coin && v1 != SingleArrow && v1 != DoubleArrow) return false; //must be a coin
	}
	Vertex v2 = getCell(i, j+2);
	if(v2==Gray) {
		grayCoins++;
		if(grayCoins > 1) return false;
		if(getCell(i+1, j) != Link || getCell(i+2, j+1) != Link) return false; //check the links
		if(coinDegree(i+2, j,  /*countArrows*/ false) != 2) return false;	//check the degree of the opposite coin
	} else {
		if(v2 != Coin && v2 != SingleArrow && v2 != DoubleArrow) return false; //must be a coin
	}
	Vertex v3 = getCell(i+2, j);
	if(v3==Gray) {
		grayCoins++;
		if(grayCoins > 1) return false;
		if(getCell(i, j+1) != Link || getCell(i+1, j+2) != Link) return false; //check the links
		if(coinDegree(i, j+2,  /*countArrows*/ false) != 2) return false;	//check the degree of the opposite coin
	} else {
		if(v3 != Coin && v3 != SingleArrow && v3 != DoubleArrow) return false; //must be a coin
	}
	Vertex v4 = getCell(i+2, j+2);
	if(v4==Gray) {
		grayCoins++;
		if(grayCoins > 1) return false;
		if(getCell(i, j+1) != Link || getCell(i+1, j) != Link) return false; //check the links
		if(coinDegree(i, j,  /*countArrows*/ false) != 2) return false;	//check the degree of the opposite coin
	} else {
		if(v4 != Coin && v4 != SingleArrow && v4 != DoubleArrow) return false; //must be a coin
	}
	if(grayCoins != 1) return false;	//we need exactly one gray coin
	
	//the canonized angle direction is the one that minimizes the distance between the gray coin and the center of the board
	if(v1==Gray) {
		//we compare (i,j) with (i+2, j+2)
		if(distanceToCenter(i+2, j+2) < distanceToCenter(i, j) ) {
			//the current orientation is not canonic : we change the orientation
			setCell(i, j, getCell(i+2, j+2));
			setCell(i, j+1, Link);
			setCell(i+1, j, Link);
			setCell(i+1, j+2, Gray);
			setCell(i+2, j+1, Gray);
			setCell(i+2, j+2, Gray);
			return true;
		}
	}
	if(v2==Gray) {
		//we compare (i,j+2) with (i+2, j)
		if(distanceToCenter(i+2, j) < distanceToCenter(i, j+2) ) {
			//the current orientation is not canonic : we change the orientation
			setCell(i, j+2, getCell(i+2, j));
			setCell(i, j+1, Link);
			setCell(i+1, j+2, Link);
			setCell(i+1, j, Gray);
			setCell(i+2, j+1, Gray);
			setCell(i+2, j, Gray);
			return true;
		}
	}
	if(v3==Gray) {
		//we compare (i,j+2) with (i+2, j)
		if(distanceToCenter(i, j+2) < distanceToCenter(i+2, j) ) {
			//the current orientation is not canonic : we change the orientation
			setCell(i+2, j, getCell(i, j+2));
			setCell(i+1, j, Link);
			setCell(i+2, j+1, Link);
			setCell(i,   j+1, Gray);
			setCell(i+1, j+2, Gray);
			setCell(i,   j+2, Gray);
			return true;
		}
	}
	if(v4==Gray) {
		//we compare (i,j) with (i+2, j+2)
		if(distanceToCenter(i, j) < distanceToCenter(i+2, j+2) ) {
			//the current orientation is not canonic : we change the orientation
			setCell(i+2, j+2, getCell(i, j));
			setCell(i+1, j+2, Link);
			setCell(i+2, j+1, Link);
			setCell(i,   j+1, Gray);
			setCell(i+1, j, Gray);
			setCell(i,   j, Gray);
			return true;
		}
	}
	
	return false;
}

//usefull function
int DotsAndBoxes::distanceToCenter(int i, int j) {
	return (rows/2-i)*(rows/2-i) + (columns/2-j)*(columns/2-j);
}

//this function is executed in SplitBoard::canonize just after splitting the board
//we canonize the linear chains
void DotsAndBoxes::gameSpecificCanonization() {
	//variables used to count some specific small boards
	int doubleArrows_number = 0;		//number of isolated double arrows
	int singleArrowPair_number = 0;		//number of singleArrow-singleArrow
	int singleDoublePair_number = 0;	//number of singleArrow-DoubleArrow
	int doubleArrowPair_number = 0;		//number of doubleArrow-doubleArrow
	
	int index;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		
		//we don't need to canonize boards that are already linear
		if(rows>1) {
			string s = canonizeCaterpillar(); //return an empty string if it is not a caterpillar
			if(!s.empty()) {
				boardList[index] = s;			//use the new (canonized) caterpillar representation 
				initCurrentBoard(index);		//initialize again the current board with the new representation
				symmetry(symmetryForCano());	//compute the best symmetry
				setCanonical(true);				//the current board is canonical
				boardList[index] = toString();	//final representation
			} else {
				//this is not a caterpillar. It can contains right angles
				if(canonizeRightAngles() ) {
					symmetry(symmetryForCano());	//compute the best symmetry
					setCanonical(true);				//the current board is canonical
					boardList[index] = toString();	//final representation
				}
			}
		}
		
		//note : test again the value of rows, because it can change in canonizeCaterpillar
		if(rows==1) {
			if(columns==1) {
				if(getCell(0, 0)==DoubleArrow) doubleArrows_number++;
			} else
			if(columns==3) {
				if(getCell(0, 0)==SingleArrow && getCell(0, 1)==Link && getCell(0, 2)==SingleArrow) {
					singleArrowPair_number++;
				} else
				if(getCell(0, 0)==DoubleArrow && getCell(0, 1)==Link && getCell(0, 2)==SingleArrow) {
					singleDoublePair_number++;
				} else
				if(getCell(0, 0)==DoubleArrow && getCell(0, 1)==Link && getCell(0, 2)==DoubleArrow) {
					doubleArrowPair_number++;
				}
			}
		}
	}
	
	//delete boards like a pair of isolated double arrows with a score variation
	deleteSmallBoards(doubleArrows_number, singleArrowPair_number, singleDoublePair_number, doubleArrowPair_number);
}

//the arguments indicate the number of existing objects in the list of boards
//there are three kinds of objects :
// * isolated DoubleArrow (red box)
// * pair of singleArrows (yellow box - link - yellow box)
// * a double with a single arrow (red box - link - yellow box)
void DotsAndBoxes::deleteSmallBoards(int doubleArrows_number, int singleArrowPair_number, int singleDoublePair_number, int doubleArrowPair_number) {
	//we can delete only pairs of these small boards
	if(doubleArrows_number%2==1) doubleArrows_number--;
	if(singleArrowPair_number%2==1) singleArrowPair_number--;
	if(singleDoublePair_number%2==1) singleDoublePair_number--;
	if(doubleArrowPair_number%2==1) doubleArrowPair_number--;
	if(doubleArrows_number==0 && singleArrowPair_number==0 && singleDoublePair_number==0 && doubleArrowPair_number==0) return;
	
	scoreVariation = doubleArrows_number/2 + singleArrowPair_number + singleDoublePair_number + doubleArrowPair_number;
	
	int index;
	vector<string> newBoardList; //new board list without the small boards
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		bool addThisBoard = true;
		
		if(rows==1) {
			if(columns==1) {
				if(doubleArrows_number>0 && getCell(0, 0)==DoubleArrow) {
					//this is an isolated DoubleArrow that needs to be deleted
					addThisBoard = false;
					doubleArrows_number--;
				}
			} else
			if(columns==3) {
				if(singleArrowPair_number >0 && getCell(0, 0)==SingleArrow && getCell(0, 1)==Link && getCell(0, 2)==SingleArrow) {
					//this is a pair of single arrows that need to be deleted
					addThisBoard = false;
					singleArrowPair_number--;
				}
				
				if(singleDoublePair_number>0 && getCell(0, 0)==DoubleArrow && getCell(0, 1)==Link && getCell(0, 2)==SingleArrow) {
					//this is a combination of double arrow and single arrow that needs to be deleted
					addThisBoard = false;
					singleDoublePair_number--;
				}
				
				if(doubleArrowPair_number>0 && getCell(0, 0)==DoubleArrow && getCell(0, 1)==Link && getCell(0, 2)==DoubleArrow) {
					//this is a pair of double arrows that needs to be deleted
					addThisBoard = false;
					doubleArrowPair_number--;
				}
			}
		}
		
		if(addThisBoard) newBoardList.push_back(boardList[index]);
	}
	
	boardList = newBoardList;
}

//======================================== Display ==========================
string DotsAndBoxes::patternForCell(int i ,int j) {
	string pattern;
	if(i==-1 && j==-1) {
		//by convention, coordinate -1, -1 is used to ask the color of the board
		pattern = "lightgray";
		return pattern;
	}
	
	Vertex v = getCell(i, j);
	if(v == Coin) {
		pattern = "blackDot";
	} else
	if(v == SingleArrow) {
		pattern = "yellowDot";
	} else
	if(v == DoubleArrow) {
		pattern = "redDot";
	} else
	if(v == Link) {
		//look for the right direction of the link
		bool end = false;
		int k;
		for(k=0; k<4 && !end; k++) {
			int i2 = i;
			int j2 = j;
			if(next(i2, j2, k)) { //move (i2, j2) in the direction of k
				//the adjacent cell in this direction is valid
				//If it is not gray, then we have found the direction of the link
				if(getCell(i2, j2) != Gray) {
					end=true;
					if(k==0 || k==1) {
						pattern = "middleHor";
					} else {
						pattern = "middleVert";
					}
				}
			}
		}
	}
	else {
		pattern = "lightgray";
	}
	return pattern;
}
