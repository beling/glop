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

#ifndef BOARD_H
#define BOARD_H

#include "../global.h"
#include "../baseclass/basegame.h"

//! @defgroup BoardGroup Board Games

//! Represent a table of n rows x m columns
//! @ingroup BoardGroup
class BoardTable {
  public: //TODO : should be private
	string internalTable;
	int rows;
	int columns;
	bool isCanonical;
	
  public:
	static const int VerticalSymmetry, HorizontalSymmetry, VerticalHorizontalSymmetry, EightSymmetries;

	BoardTable();
	BoardTable(string init);
	string getString();
	void applyTransformation(int sym); //0 = vert sym, 1=hor sym, 2=1/4 rot
	void minimumSymmetry(int sym);
	
	bool operator<(const BoardTable& a) const;
};

//! Base Class for representing a Board Game
//! @ingroup BoardGroup
class Board : public BaseGame {
  private:
	BoardTable board_rep;  //!< current board
	Vertex playerToPlay; //Black or White
	
	BoardTable boardSav;
	Vertex playerSav;
	int rowsSav;
	int columnsSav;
	
  public:
	vector<string> boardList; //!< vector of all boards
	
	//inherited from BaseGame
	virtual void resetFrom(const Line &a);		//!< reset the object
	virtual void resetFrom(const string &a);	//!< reset the object
	virtual string exportToString();			//!< string representation of the list of boards
	virtual Line exportToLine();				//!< line representation of the list of boards
	virtual Interface getParamDef();
	virtual void setParam();
	virtual void paint(DisplayPainter &painter);	//!< draw a graphic representation of the board
	
	//TODO : this function is not beautiful
	virtual Line exportToLineReverse(); //! line representation and reverse the next player
	
	Board();
	
	//static members to describe parameters and constants
	static int boardRows;
	static int boardColumns;
	
	static const Vertex Black, White, Coin, Link, SingleArrow, DoubleArrow, Gray, Empty;
	static const Vertex SizeMark, EndBoard, EndCanonicalBoard, EndGameSpecific;
	static const Vertex ColoredCoin, ColoredLink, ColoredSingleArrow;
	
	static bool isClassicPosition; //usually used in the parameter interface
	static string givenPosition;  //usually used in the parameter interface
	
	static bool distinguishPlayers;
	
	//public interface
	int rows;
	int columns;
	
	//getCell-setCell functions. Theses functions are "inline" because they are used constantly
	inline Vertex getCell(int i, int j) { //get the Vertex at i,j
		return getCell(i, j, board_rep.internalTable);
	}
	inline void setCell(int i, int j, Vertex a) { // set the vertex at i, j
		setCell(i, j, a, board_rep.internalTable);
	} 
	inline Vertex getCell(int i, int j, string& s) {
		return getCell(i, j, s, columns);
	}
	inline void setCell(int i, int j, Vertex a, string& s) {
		setCell(i, j, a, s, columns);
	}
	inline Vertex getCell(int i, int j, string& s, int columnsNumber) {
		if(i*columnsNumber + j < (int) s.length()) {
			return s[i*columnsNumber + j];
		}
		return 0; //error case
	}
	inline void setCell(int i, int j, Vertex a, string& s, int columnsNumber) {
		if(i*columnsNumber + j < (int) s.length()){
			s[i*columnsNumber + j] = (unsigned char) a;
		}
	}
	
	inline bool validate(int i, int j) { //return true if the coordinate are valid
		if(i<0 || j<0 || i>rows - 1 || j > columns - 1) {
			return false;
		} else {
			return true;
		}
	}
	
	//usefull function to implement concept of horizontal, vertical or diagonal
	//! Modify i and j in order to "move" to the next cell in the given direction.
	//! Directions : 0 = down, 1 = up, 2 = right, 3 = left, 4-7 : diagonals
	//! NOTE : contrary to the previous version, this function does nothing if the move is not valid
	//! @return true if the move is possible
	inline bool next(int&i, int&j, int direction) { 
		switch(direction) {
			case 0 : if(i>=rows-1)    return false; i++; return true;
			case 1 : if(i<=0)         return false; i--; return true;
			case 2 : if(j>=columns-1) return false; j++; return true;
			case 3 : if(j<=0)         return false; j--; return true;
			case 4 : if(i>=rows-1||j>=columns-1) return false; i++; j++; return true;
			case 5 : if(i>=rows-1||j<=0)         return false; i++; j--; return true;
			case 6 : if(i<=0     ||j>=columns-1) return false; i--; j++; return true;
			case 7 : 
			default : if(i<=0    ||j<=0)         return false; i--; j--; return true;
		}
	}
	
	inline Vertex reverseColor(Vertex a) {
		if(a == Black) return White;
		if(a == White) return Black;
		return a;
	}
	
	inline void reversePlayer() {//set playerToPlay to the next player
		playerToPlay=reverseColor(playerToPlay);
	}
	
	inline Vertex currentPlayer() {//get the current player
		return playerToPlay;
	}
	
	void reverseAllBoardsPlayer(); //reverse the color of the next player in all the boards of the list
	
	//!< reduce the size of the board to a given rectangle
	void reduceBoardTo(int i0, int j0, int i1, int j1);
	
	//functions to save the board
	void saveBoard();
	void backToSavedBoard();
	
	//symmetries
	void applyTransformation(int sym, list<BoardTable>& input, list<BoardTable>& output); //0 = vert sym, 1=hor sym, 2=1/4 rot
	void generateSymmetry(int sym, list<BoardTable>& result); //0 = vert sym, 1=hor sym, 2=1/4 rot, 3= vert + hor, 4=all
	void symmetry(int sym);
	int collisions(const string& A, const string& B); //number of similar letters in the two string
	int symmetryDegree(int sym); //return the "degree" of symmetry of the board
	bool isSymmetric(int sym);
	
	//conversions between string and Line types
	string toString(); //!< string representation of the current board
	Line toLine();  //!< line representation of the current board
	void fromString(const string& stringA); //!< init the list of boards from a string
	void fromLine(const Line& lineA);  //!< init the list of boards from a Line
	void initCurrentBoard(int index); //!< init the current board with boardList[index]
	void initEmptyBoard();
	virtual string gameSpecificString();	//!< can be used by a game to add its specific data to the string representation
	virtual void initGameSpecific(const string& stringA);
	
	//get-set the isCanonical flag
	void setCanonical(bool isCanonical);
	bool isCanonical();
	
	//----------- display --------------
	string coutString();	//cout the board string with carriage returns
	void drawBoard(DisplayPainter &painter);		//main function to draw a list of boards
	void paintCurrentBoard(DisplayPainter &painter, int xPos); //draw the current board
	virtual string patternForCell(int i, int j);		//return the name of the display pattern for a given cell
};

#endif
