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

#ifndef GAMEGLOBAL_H
#define GAMEGLOBAL_H

#include "global.h"

//takes the int 273 and returns the string "273"
string int_to_string(const unsigned int& n);

//global constant values (we prefer class static members instead of #define)
class global {
  public :
	//maximum number of levels displayed in the interface
	static const int Max_Level;
	
	//nimber and win_loss special values
	//the following Computation_end should be deleted in the end
	static const nimber Nim_unknown;
	static const win_loss Loss, Win, Draw, WinOrDraw;
	
	static string toString(win_loss w);
	
	//check computation : values + random function
	static const win_loss CheckedUnknown, CheckedLoss, CheckedWin;
	static win_loss randomCheck(win_loss knownValue);
	
};

//class for the types of games
class GameType {
  public:
	//types of games
	static int ImpartialGame, PartizanGameWL, CombinatorialGameWLD, NimSeqGame, ScoreGame;
	static int NumberOfTypes;
	
	//variable used by a Node to indicate if it can be used for a type of game
	vector<bool> isFor;
	GameType();
	void add(int a); //add a type of games for the node
};

class Parameter {
  public:
	//start position parameters
	//TODO : in the end, given_position and usePositionString should disappear
	static Line given_position;		//user-defined starting position (position part)
	static PositionString given_positionString; //user-defined starting position (PositionString type)
	static bool usePositionString;			//temporary variable to distinguish between Line and PositionString
	
	//kind of algorithm
	static int gameChoice;			//game selected by the user (Sprouts, Cram...)
	static int nodeChoice;			//node selected by the user (WinLoss, Nimber...)
	static int computation_kind;	//kind of computation inside the node
	static int traversalChoice;		//game tree traversal selected by the user (Alpha-beta, Proof-number search...)
	
	static bool isCheck;
	static int RandomCheckCycles;
	static bool LastCycleIsNormal;
	
	static bool useCompression;
	
	//life limit for Rct switch in the misere complex algorithm
	static int lifeLim;
	
	//special value to indicate that we are converting files
	static bool isConvertingFiles;
	
	//the following value is used by nodes to activate debug infos in games
	static bool debugGames;
};


//======================Class used for games with a notion of score =================

typedef unsigned char Score;

//usefull class to represent a range of possible scores
class ScoreRange {
  public:
	Score minPossibleScore;
	Score maxPossibleScore;
	
	ScoreRange();
	void increaseMinTo(Score newMin);
	void decreaseMaxTo(Score newMax);
	
	string toString();			//return a string representation of the range
	void fromString(string s);	//initialize the range from a given string
	void mergeString(string s);	//update the range to take into account another range represented as a string
	
	static const Score Score_max;
};

#endif
