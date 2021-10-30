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

#include "board.h"
#include "../displaypainter.h"

int Board::boardRows = 4;
int Board::boardColumns = 4;
bool Board::isClassicPosition=true;
string Board::givenPosition;
bool Board::distinguishPlayers=true;

const Vertex Board::Empty='0';
const Vertex Board::Black='B';
const Vertex Board::White='W';
const Vertex Board::Link='L';
const Vertex Board::DoubleArrow='A';
const Vertex Board::SingleArrow='B';
const Vertex Board::Coin='C';
const Vertex Board::Gray='G';
const Vertex Board::SizeMark='*';
const Vertex Board::EndBoard='E';
const Vertex Board::EndCanonicalBoard='F';
const Vertex Board::EndGameSpecific='K';

const Vertex Board::ColoredCoin='U';
const Vertex Board::ColoredSingleArrow='V';
const Vertex Board::ColoredLink='M';

//names for the different kind of symmetries
const int BoardTable::VerticalSymmetry=0;
const int BoardTable::HorizontalSymmetry=1;
const int BoardTable::VerticalHorizontalSymmetry=2;
const int BoardTable::EightSymmetries=4;

//---------------------------class BoardTable--------------------------
//strings representing BoardTable should be of the form xxx*xxxE
//Board::SizeMark(*) indicates the end of the first row
//Board::EndBoard(E) is the terminal character

BoardTable::BoardTable() {
	rows = 0;
	columns = 0;
	isCanonical = false;
}

//compare operator for BoardTable
bool BoardTable::operator<(const BoardTable& a) const{
	//always choose the minimal number of rows
	if(rows!=a.rows) {
		return (rows < a.rows);
	}
	
	//minimal number of columns
	if(columns!=a.columns) {
		return (columns < a.columns);
	}
	
	//for equal number of rows and columns, choose the minimal representation
	return (internalTable < a.internalTable);
}

BoardTable::BoardTable(string init) {
	rows = 0;
	columns = 0;
	
	int i;
	unsigned char c;
	int n= init.size();
	
	if(n<0) return; //this is a void board; TODO : what is this case ??
	
	bool correct = false;
	for(i=0;i<n;i++) {
		c=init[i];
		if(c==Board::SizeMark) {
			//this is the special character indicating the table size
			//in case of more than one line
			columns = i;
			
			//check that the size is correct : there are two special characters (*, E)
			if( (n-2) % columns == 0) {
				correct = true;
				rows = (n-2)/columns;
			}
		} else
		if(c==Board::EndBoard) {
			isCanonical = false;
		} else
		if(c==Board::EndCanonicalBoard) {
			isCanonical = true;
		} else {
			internalTable += c;
		}
	}
	
	if(!correct && n>1) {
		cout << "incorrect Size in BoardTable::BoardTable : " << init << endl;
	}
}

string BoardTable::getString() {
	string result = internalTable;
	if(columns > 0 && columns <= (int) result.size()) {
		string a; a+= Board::SizeMark; //minor hack because of invalid conversion warning
		result.insert(columns, a);
	} else {
		cout << "getString error - Rows : " << rows << " Columns : " << columns << " " << internalTable << endl;
	}
	
	if(isCanonical) {
		result += Board::EndCanonicalBoard;
	} else {
		result += Board::EndBoard;
	}
	return result;
}

//Apply a transformation to the table 
//transformation : 0=vertical symmetry,  1=horizontal symmetry, 2= 1/4 rotation
void BoardTable::applyTransformation(int sym) {
	int i, j;
	string newTable = internalTable; //intialization to the same size as *it
	
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			if(sym==0) {
				//vertical symmetry : copy (i,j) in (i, columns - 1 - j)
				newTable[i*columns + columns - 1 - j] = internalTable[i*columns + j];
			} else
			if(sym==1) {
				//horizontal symmetry : copy (i,j) in (rows- 1 - i, j)
				newTable[(rows - 1 - i)*columns + j] = internalTable[i*columns + j];
			} else
			if(sym==2) {
				//rotation : copy (i,j) in (j, rows - 1 - i)
				//rotation change the number of columns
				//the number of columns for newTable is the number of rows for internalTable
				newTable[j*rows + rows - 1 - i] = internalTable[i*columns + j];
			}
		}
	}
	
	internalTable = newTable;
	
	if(sym==2) {
		int a = rows;
		rows = columns;
		columns = a;
	}
}

//find quickly the best symmetry -- not used -- still in TESTING
//compare directly the correct cells of the current board
//there are 8 ways to go through the board (4 corners, 2 directions)
//the argument sym indicates the authorized symmetry
void BoardTable::minimumSymmetry(int sym) {
	int i=0, j=0, i2=0, j2=0, p=0, k=0;
	bool symCandidate[8]; unsigned char symValue[8]; unsigned char minValue;
	int candidateNumber = 8;
	for(k=0; k<8; k++) {symCandidate[k]=true;}

	if(sym==VerticalSymmetry || sym==HorizontalSymmetry || sym==VerticalHorizontalSymmetry) {
		//all 1/4 rotations are forbidden
		symCandidate[4]=false;
		symCandidate[5]=false;
		symCandidate[6]=false;
		symCandidate[7]=false;
	}
	if(sym==VerticalSymmetry) {
		symCandidate[1]=false; //horizontal forbidden
		symCandidate[3]=false; //1/2 rotation forbidden
	}
	if(sym==HorizontalSymmetry) {
		symCandidate[2]=false; //vertical forbidden
		symCandidate[3]=false; //1/2 rotation forbidden
	}
	
	//keep the orientation that gives the less number of rows 
	if(rows<columns) {
		//it is already the best number of rows, so we don't authorize to change the role of rows and columns
		//i.e 1/4 rotation are not authorized
		symCandidate[4]=false;
		symCandidate[5]=false;
		symCandidate[6]=false;
		symCandidate[7]=false;
		candidateNumber = candidateNumber -4;
	}
	
	if(columns<rows) {
		//we need to change the role of rows and columns , so we absolutely need a 1/4 rotation
		//except if the authorized symetries don't allow to exchange rows for columns
		if(symCandidate[4]==true || symCandidate[5]==true || symCandidate[6]==true || symCandidate[7]==true) {
			symCandidate[0]=false;
			symCandidate[1]=false;
			symCandidate[2]=false;
			symCandidate[3]=false;
			candidateNumber = candidateNumber -4;
		}
	}
	
	//stop when there is only one candidate symmetry left or when the whole board has been checked
	for(p=0; p<rows*columns && candidateNumber>1; p++) {
		//find the next character for the 8 possible symetries
		symValue[0]=internalTable[      i    *columns + j             ]; //current board
		symValue[1]=internalTable[ (rows-1-i)*columns + j             ]; //horizontal symmetry
		symValue[2]=internalTable[      i    *columns + (columns-1-j) ]; //vertical symmetry
		symValue[3]=internalTable[ (rows-1-i)*columns + (columns-1-j) ]; //1/2 rotation
		symValue[4]=internalTable[     i2    *columns + j2            ]; //1/4 rotation followed by vertical symmetry
		symValue[5]=internalTable[(rows-1-i2)*columns + j2            ]; //1/4 rotation
		symValue[6]=internalTable[     i2    *columns + (columns-1-j2)]; //1/4 rotation followed by 1/2 rotation
		symValue[7]=internalTable[(rows-1-i2)*columns + (columns-1-j2)]; //1/4 rotation followed by horizontal symmetry
		
		//compute the minValue
		minValue = 255;
		for(k=0; k<8; k++) {
			if(symCandidate[k]) {
				if(symValue[k] < minValue) minValue=symValue[k];
			}
		}
		
		//keep as candidates only the symmetries still equal to minValue
		for(k=0; k<8; k++) {
			if(symCandidate[k] && symValue[k] != minValue) {
				symCandidate[k]=false;
				candidateNumber--; //decrement the number of candidates
			}
		}
		
		//increment i, j, i2, j2
		j++;  if(j==columns) {i++;   j=0;}
		i2++; if(i2==rows)   {j2++; i2=0;}
	}
	
	//find the remaining candidate
	int best=0;
	for(k=0; k<8; k++) {
		if(symCandidate[k]==true) {
			best = k;
		}
	}
	
	if(best==0) return; //the current board is the best one
	
	//compute the new board for this symmetry
	string newTable = internalTable; //intialization to the same size as *it
	
	i2=0; j2=0;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			if(best==1) { //copy (rows-1-i, j) in (i,j)
				newTable[i*columns + j] = internalTable[(rows-1-i)*columns + j];
			} else
			if(best==2) { //copy (i, columns-1-j) in (i,j)
				newTable[i*columns + j] = internalTable[i*columns + (columns-1-j)];
			} else
			if(best==3) { //copy (rows-1-i, columns-1-j) in (i,j)
				newTable[i*columns + j] = internalTable[(rows-1-i)*columns + (columns-1-j)];
			} else
			if(best==4) { //copy (i2, j2) in (i,j)
				newTable[i*columns + j] = internalTable[ i2*columns + j2  ];
			} else
			if(best==5) { //copy (rows-1-i2, j2) in (i,j)
				newTable[i*columns + j] = internalTable[(rows-1-i2)*columns + j2];
			} else
			if(best==6) { //copy (i2, columns-1-j2) in (i,j)
				newTable[i*columns + j] = internalTable[i2    *columns + (columns-1-j2)];
			} else
			if(best==7) { //copy (rows-1-i2, columns-1-j2) in (i,j)
				newTable[i*columns + j] = internalTable[(rows-1-i2)*columns + (columns-1-j2)];
			}
			
			i2++; if(i2==rows) {j2++; i2=0;}
		}
	}
	
	internalTable = newTable;
	if(best>=4) {
		int a = rows;
		rows = columns;
		columns = a;
	}
}

//---------------------------class Board--------------------------
Board::Board() {
	//by default, use BoardRows and BoardColumns as board dimensions
	rows = boardRows;
	columns = boardColumns;
}

//reverse the color of the next player in all the boards of the list
void Board::reverseAllBoardsPlayer() {
	if(!distinguishPlayers) return;  //nothing to do if we don't distinguish the players
	
	int i;
	for(i = 0; i < (int) boardList.size(); i++) {
		if(!boardList[i].empty()) boardList[i][0] = reverseColor(boardList[i][0]);
	}
}

//------------inherited from BaseGame------------------------
void Board::resetFrom(const Line &a) {
	fromLine(a);
}

void Board::resetFrom(const string &a) {
	fromString(a);
}

//string representation of the list of boards
string Board::exportToString() {
	string result = gameSpecificString(); //store first the specific data of the game (if any)
	if(!result.empty()) {
		result += EndGameSpecific;
	}
	int i;
	for(i = 0; i < (int) boardList.size(); i++) {
		result += boardList[i];
	}
	
	return result;
}

//can be reimplemented by a game to add its specific data to the string representation
//the default implementation returns an empty string
string Board::gameSpecificString() {
	string empty;
	return empty;
}

//line representation of the list of boards
//note : will soon become obsolete
Line Board::exportToLine() {
	Line result;
	
	int i;
	for(i = 0; i < (int) boardList.size(); i++) {
		//initialize the current board
		initCurrentBoard(i);
		Line tmp = toLine();
		result.insert(result.end(), tmp.begin(), tmp.end());
	}
	
	return result;
}

//! line representation and reverse the next player
//note : will soon become obsolete
Line Board::exportToLineReverse() {
	Line result;
	
	int i;
	for(i = 0; i < (int) boardList.size(); i++) {
		//initialize the current board
		initCurrentBoard(i);
		reversePlayer();
		Line tmp = toLine();
		result.insert(result.end(), tmp.begin(), tmp.end());
	}
	
	return result;
}

//Usual parameter interface for a board
Interface Board::getParamDef() {
	Interface result;
	result.name="Board";
	
	result.addLabel("Board rows :", /*pos*/ 0, 0);
	result.addSpinBox(1, 30, 4, /*pos*/ 0, 1);
	result.link(Board::boardRows, "BoardRows");
	
	result.addLabel("Board columns :", /*pos*/ 0, 2);
	result.addSpinBox(1, 30, 7, /*pos*/ 0, 3);
	result.link(Board::boardColumns, "BoardColumns");
	
	result.addRadioButton("Classic empty position", true, /*pos*/ 1, 0);
	result.link(Board::isClassicPosition, "isClassicPosition");
	result.addRadioButton("Given start position", false, /*pos*/ 2, 0);
	
	Board b;
	b.initEmptyBoard();
	result.addLineEdit(b.toString(), /*pos*/ 2, 1, 1, 3);
	result.link(Board::givenPosition, "GivenPosition");
	
	return result;
}

void Board::setParam() {
	//start position
	Board b;
	if(!Board::isClassicPosition) {
		b.fromString(Board::givenPosition);	//given start position
	} else {
		b.initEmptyBoard();
	}
	Parameter::usePositionString = false;
	Parameter::given_position=b.toLine();
}

// ------------ String/Line operations, List of Boards ----------------

//string representation of the current board
string Board::toString() {
	string result;
	if(distinguishPlayers) result += playerToPlay;
	result += board_rep.getString();
	return result;
}

//line representation of the current board
Line Board::toLine() {
	Line result;
	if(distinguishPlayers) result.push_back(playerToPlay);
	
	string s = board_rep.getString();
	int i; int n = s.length();
	for(i=0; i<n; i++) {
		result.push_back(s[i]);
	}
	
	return result;
}


//init to an empty board (with black to play if we distinguish players)
void Board::initEmptyBoard() {
	string init;
	if(distinguishPlayers) init += Black; //black begins
	
	int i;
	for(i=0; i<boardRows*boardColumns; i++) {
		init += Empty;
		if(i==boardColumns-1) {
			init += SizeMark;
		}
	}
	init += EndBoard;
	fromString(init);
}

//init the list of boards from a string
//boardList is initialized with the list of boards
//the internal current board is initialized with the first board of the list
void Board::fromString(const string& stringA) {
	boardList.clear();
	int i;
	int n=stringA.length();
	unsigned char c;
	string oneString;
	for(i=0; i<n; i++) {
		c=stringA[i];
		if(c==EndBoard || c==EndCanonicalBoard) {
			//this is the end of one board
			oneString += c;
			boardList.push_back(oneString);
			oneString.clear();
		} else
		if(c==EndGameSpecific) {
			//this is the end of the specific data of the game
			initGameSpecific(oneString);
			oneString.clear();
		} else {
			oneString += c;
		}
	}
	
	if(!boardList.empty()) initCurrentBoard(0);
}

//default implementation for specific data of a game (nothing to do)
void Board::initGameSpecific(const string& /*stringA*/) {
	
}

//init the list of boards from a Line
//note : will soon become obsolete
void Board::fromLine(const Line& lineA) {
	boardList.clear();
	Line::const_iterator s;
	unsigned char c;
	string oneString;
	for(s=lineA.begin(); s!=lineA.end(); s++) {
		c=(unsigned char) (*s);
		oneString += c;
		if(c==EndBoard || c==EndCanonicalBoard) {
			//this is the end of one board
			boardList.push_back(oneString);
			oneString.clear();
		}
	}
	
	if(!boardList.empty()) initCurrentBoard(0);
}

//initialize the current board with boardList[index]
void Board::initCurrentBoard(int index) {
	board_rep = BoardTable();
	rows = 0;
	columns = 0;
	
	//security
	if(index < 0 || index >= (int) boardList.size() ) return;
	int n=boardList[index].length();
	if(n<1) return ; //this is a void board
	
	string s = boardList[index];
	if(distinguishPlayers) {
		playerToPlay = boardList[index][0];
		s.erase(0,1); //erase the first character (player to play)
	}
	board_rep=BoardTable(s); //init board_rep with the remaining string
	rows = board_rep.rows;
	columns = board_rep.columns;
}

string Board::coutString() {
	string result;
	Line::const_iterator s;
	
	if(distinguishPlayers) result += playerToPlay;
	
	int i; int n = board_rep.internalTable.length();
	for(i=0; i<n; i++ ) {
		if(i%columns==0) {
			result += "\n";
		}
		
		result += board_rep.internalTable[i];
	}
	
	result += "\n";
	
	return result;
}


void Board::saveBoard() {
	boardSav = board_rep;
	playerSav = playerToPlay;
	rowsSav = rows;
	columnsSav = columns;
}

void Board::backToSavedBoard() {
	board_rep = boardSav;
	playerToPlay = playerSav;
	rows = rowsSav;
	columns = columnsSav;
}

// ================== Symmetry =======================

//generate the list of boards obtained by applying a transformation to all the positions of the list
//the resulting boards are added at the end of the input list
//transformations : 0=vertical symmetry,  1=horizontal symmetry, 2= 1/4 rotation

void Board::applyTransformation(int sym, list<BoardTable>& input, list<BoardTable>& output) {
	list<BoardTable>::iterator it;
	for(it=input.begin(); it!=input.end(); it++) {
		BoardTable boardV = *it; //intialization to *it
		boardV.applyTransformation(sym); //apply the transformation
		output.push_back(boardV);
	}
}

//Generate the boards obtained with some transformations
//0=vertical symmetry, 1=horizontal symmetry, 2=horizontal+vertical, 3=rotations, 4=all
void Board::generateSymmetry(int sym, list<BoardTable>& result) {
	result.push_back(board_rep);
	
	list<BoardTable> partialResult;
	
	if(sym==0 || sym==1 || sym==3) {
		applyTransformation(sym, result, partialResult);
		result.insert(result.end(), partialResult.begin(), partialResult.end());
	}
	if(sym==2) {
		applyTransformation(0, result, partialResult);
		result.insert(result.end(), partialResult.begin(), partialResult.end());
		applyTransformation(1, result, partialResult);
		result.insert(result.end(), partialResult.begin(), partialResult.end());
	}
	if(sym==4) {
		applyTransformation(0, result, partialResult);
		result.insert(result.end(), partialResult.begin(), partialResult.end());
		applyTransformation(1, result, partialResult);
		result.insert(result.end(), partialResult.begin(), partialResult.end());
		applyTransformation(2, result, partialResult);
		result.insert(result.end(), partialResult.begin(), partialResult.end());
	}
}

//find the best symmetry for lexicographical order
//the following function compute the canonic form of the board for a given symmetry
//possible symmetries : BoardTable::VerticalSymmetry, HorizontalSymmetry, VerticalHorizontalSymmetry, EightSymmetries
void Board::symmetry(int sym) {
	board_rep.minimumSymmetry(sym);
	rows = board_rep.rows;
	columns = board_rep.columns;
}

//number of similar letters in the two strings
int Board::collisions(const string& A, const string& B) {
	int r = 0;
	int nA = A.size();
	int nB = B.size();
	int i;
	for(i=0; i<nA && i<nB; i++) {
		if(A[i] == B[i]) r++;
	}
	return r;
}

int Board::symmetryDegree(int sym) {
	list<BoardTable> candidates;
	generateSymmetry(sym, candidates);
	if((int) candidates.size() < 2) return 0;
	candidates.pop_front();
	
	//count the number of similar cells between the board and each candidate
	int total=0;
	list<BoardTable>::iterator Si;
	for(Si=candidates.begin(); Si!=candidates.end(); Si++) {
		total += collisions(board_rep.internalTable, Si->internalTable);
	}
	
	//to avoid to distinguish the boards too much return a number between 0 and 3
	//we simply compare the number of collisions to the maximum possible number
	int max = board_rep.internalTable.size()*candidates.size();
	if(max==0) {
		cout << "Warning : call of Board::symmetryDegree with an incorrect board" << endl;
		cout << "boarlist.size : " << boardList.size() << endl;
		max=1; //to avoid a crash
	}
	int result = (total * 3) / max;
	return result;
}

//indicate if a board is symmetric
bool Board::isSymmetric(int sym) {
	list<BoardTable> candidates;
	generateSymmetry(sym, candidates);
	
	//the board must be exactly the same as all the symmetries
	list<BoardTable>::iterator Si;
	for(Si=candidates.begin(); Si!=candidates.end(); Si++) {
		if(board_rep.internalTable != Si->internalTable) return false;
	}
	
	return true;
}

void Board::reduceBoardTo(int i0, int j0, int i1, int j1) {
	if(i0<=i1 && j0<=j1) {
		string newBoard;
		int i, j;
		for(i=i0; i<=i1; i++) {
			for(j=j0; j<=j1; j++) {
				newBoard += getCell(i, j);
			}
		}
		
		board_rep.internalTable=newBoard;
		board_rep.rows = i1-i0+1;
		board_rep.columns = j1-j0+1;
		rows = i1-i0+1;
		columns = j1-j0+1;
	} else {
		//void board
		//use a board with only one gray cell until code is checked
		//TODO : check this code
		string a;
		a += Gray;
		board_rep.internalTable = a;
		board_rep.rows = 1;
		board_rep.columns = 1;
		rows = 1;
		columns = 1;
	}
}

//Get-set the isCanonical flag
void Board::setCanonical(bool isCanonical) {
	board_rep.isCanonical = isCanonical;
}

bool Board::isCanonical() {
	return board_rep.isCanonical;
}

//====================== Graphic representation of the board ==================
//Draw a graphic representation of the board
//By default, we simply call drawBoard
//This function can be reimplemented in the derived classes
void Board::paint(DisplayPainter &painter) {
	if(painter.computeHeightOnly) {
		//the displayPainter requests only a computation of the height
		painter.neededHeight = 0;
		int index;
		for(index = 0; index < (int) boardList.size(); index++) {
			initCurrentBoard(index); //initialize the current board
			int verSize = painter.cellSize*rows;
			if(verSize > painter.neededHeight) painter.neededHeight = verSize;
		}
	} else {
		//draw the board in the displayPainter
		drawBoard(painter);
	}
}

//main function to draw a board
void Board::drawBoard(DisplayPainter &painter) {
	int cellSize = painter.cellSize;
	int index; int xPos=0;
	for(index = 0; index < (int) boardList.size(); index++) {
		initCurrentBoard(index); //initialize the current board
		int horSize = cellSize*columns;
		int verSize = cellSize*rows;
		
		painter.setClipRect(xPos, 0, horSize+1, verSize+1);
		string boardColor = patternForCell(-1, -1);
		painter.fillRect(xPos, 0, horSize+1, verSize+1, boardColor);
		
		paintCurrentBoard(painter, xPos);
		xPos += horSize + cellSize;
	}
}

void Board::paintCurrentBoard(DisplayPainter &painter, int xPos) {
	int cellSize = painter.cellSize;
	int i, j;
	for(i=0; i<rows; i++) {
		for(j=0; j<columns; j++) {
			string pattern = patternForCell(i, j);
			if(pattern == "middleHor" || pattern == "middleVert") {
				//painter.fillRect(xPos + cellSize*j+1, cellSize*i+1, /*width*/ cellSize-1, /*height*/ cellSize-1, "gray");
				if(pattern=="middleHor") {
					painter.drawLine(xPos+cellSize*j+cellSize/2+1, cellSize*i+1, xPos+cellSize*j+cellSize/2+1, cellSize*(i+1)+2, "black", 2);
				} else {
					painter.drawLine(xPos+cellSize*j+1, cellSize*i+cellSize/2+2, xPos+cellSize*(j+1)+2, cellSize*i+cellSize/2+2, "black", 2);
				}
			} else
			if(pattern == "blackDot") {
				painter.fillRect(xPos + cellSize*j+3, cellSize*i+3, /*width*/ cellSize-3, /*height*/ cellSize-3, "black");
			} else
			if(pattern == "yellowDot") {
				painter.fillRect(xPos + cellSize*j+3, cellSize*i+3, /*width*/ cellSize-3, /*height*/ cellSize-3, "yellow");
			} else
			if(pattern == "redDot") {
				painter.fillRect(xPos + cellSize*j+3, cellSize*i+3, /*width*/ cellSize-3, /*height*/ cellSize-3, "red");
			} else {
				painter.fillRect(xPos + cellSize*j+1, cellSize*i+1, /*width*/ cellSize-1, /*height*/ cellSize-1, pattern);
			}
		}
	}
}

//main default patterns (colors) of the different kind of cells
string Board::patternForCell(int i, int j) {
	string pattern;
	if(i==-1 && j==-1) {
		//by convention, coordinate -1, -1 is used to ask the color of the board
		pattern = "black";
		return pattern;
	}
	
	Vertex v = getCell(i, j);
	if(v == Empty) {
		pattern = "white";
	} else
	if(v == Black) {
		pattern = "red";
	} else
	if(v == White) {
		pattern = "blue";
	} else {
		pattern = "gray";
	}
	return pattern;
}
