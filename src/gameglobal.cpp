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

//Constant values and functions used in the game class

#include "gameglobal.h"
#include "stringconverter.h"

//----------game categories-----------------------------------------------------------
int GameType::ImpartialGame=0;
int GameType::PartizanGameWL=1;
int GameType::CombinatorialGameWLD=2;
int GameType::NimSeqGame=3;
int GameType::ScoreGame=4;
int GameType::NumberOfTypes=5; //indicate the number of types for the function GameType::GameType

//initialize the vector size
//false is the default for all types
GameType::GameType() {
	isFor.resize(NumberOfTypes);
	int i;
	for(i=0; i<NumberOfTypes; i++) {
		isFor[i] = false;
	}
}

void GameType::add(int a) {
	if(a>=0 && a<NumberOfTypes) {
		isFor[a] = true;
	}
}

//----------graphical interface--------------------------------------------------------------------
//maximum number of displayed level
const int global::Max_Level=40;

//----------nimber and win_loss--------------------------------------------------------------------

//unknown nimber
const nimber global::Nim_unknown=254;

//win_loss
const win_loss global::Loss=0;
const win_loss global::Win=1;
const win_loss global::Draw=252;
const win_loss global::WinOrDraw=253;

string global::toString(win_loss w) {
	if(w == global::Loss) {
		return string("Loss");
	} else if(w == global::Win) {
		return string("Win");
	} else if(w == global::Draw) {
		return string("Draw");
	} else {
		return string("WinOrDraw");
	}
}

const win_loss global::CheckedUnknown=1;
const win_loss global::CheckedWin=51;
const win_loss global::CheckedLoss = 101;

//knownValue = known value of a node in a previous computation (win, loss, etc.)
//return a value to order correctly the nodes in the check computation
//random order allows us to prune useless positions in solution trees
// We use losing child > winning child > unknown child
//For some algorithms, it is important to distinguish between "win" and "unknown", for example in the case of sum of components (nimbernode)
win_loss global::randomCheck(win_loss knownValue) {
	win_loss result;
	
	//note : the last cycle of a random check is always a "normal check" (without randomness)
	//in order to obtain a reproducible database with normal check
	if(Parameter::RandomCheckCycles>1 || (Parameter::RandomCheckCycles==1 && !Parameter::LastCycleIsNormal) ) {
		if(knownValue==global::Loss) {
			result = CheckedLoss+(rand()%100);
		} else
		if(knownValue==global::Win) {
			result = CheckedWin+(rand()%50);
		} else {
			result = CheckedUnknown+(rand()%50);
		}
	} else {
		if(knownValue==global::Loss) {
			result = CheckedLoss;
		} else
		if(knownValue==global::Win) {
			result = CheckedWin;
		} else {
			result = CheckedUnknown;
		}
	}
	
	return result;
}


//takes the int 273 and returns the string "273"
//TODO : y'a sans doute bien plus efficace !
string int_to_string(const unsigned int& n) {
	string result;
	if(n==0){
		result += '0';
		return result;
	}
	unsigned int n_tmp=n;
	list<unsigned char> l_tmp;
	while(n_tmp!=0){
		l_tmp.push_front(n_tmp%10);
		n_tmp /= 10;
	}
	list<unsigned char>::iterator Si;
	for(Si=l_tmp.begin();Si!=l_tmp.end();Si++){
		result += (unsigned char) (*Si + '0');
	}
	return result;
}

//parameter class static members
int Parameter::nodeChoice;				//indicate which node is used
int Parameter::gameChoice;				//indicate which game is used
int Parameter::traversalChoice=0;		//indicate which traversal is used
Line Parameter::given_position;			//user-defined starting position (position part)
PositionString Parameter::given_positionString; 
bool Parameter::usePositionString = false;
int Parameter::computation_kind;		//kind of computation (cf gameglobal.cpp)
int Parameter::lifeLim=0; 				//life limit for Rct switch in the misere complex algorithm
bool Parameter::isCheck=false;
int Parameter::RandomCheckCycles=0;
bool Parameter::LastCycleIsNormal=false;
bool Parameter::useCompression=false;

//special value to indicate that we are converting files
bool Parameter::isConvertingFiles=false;

//the following value is used by nodes to activate debug infos in games
bool Parameter::debugGames=false;


//=========================================================================================
//================================= Score Range ===========================================

const Score ScoreRange::Score_max=250; //maximal value of a valid score

ScoreRange::ScoreRange() {
	minPossibleScore = 0;
	maxPossibleScore = Score_max+1; //convention to indicate that max is not set yet
}

void ScoreRange::increaseMinTo(Score newMin) {
	if(newMin > minPossibleScore) {
		minPossibleScore = newMin;
	}
}

void ScoreRange::decreaseMaxTo(Score newMax) {
	if(newMax < maxPossibleScore) {
		maxPossibleScore = newMax;
	}
}

string ScoreRange::toString() {
	StringConverter conv;
	
	//add the minimum possible score to the string only if it is not null
	if(minPossibleScore !=0) {
		conv << minPossibleScore << string(";");
	}
	conv << maxPossibleScore;
	return conv.getString();
}

void ScoreRange::fromString(string s) {
	StringConverter conv(s);
	conv >> CvCd("UT") >> CvCd("E;"); //use a separator between elements, set the separator to ";"
	if(conv.tokenNumber() > 1) {
		conv >> minPossibleScore;
	}
	conv >> maxPossibleScore;
}

//update the range to take into account another range represented as a string
void ScoreRange::mergeString(string s) {
	ScoreRange range;
	range.fromString(s);
	increaseMinTo(range.minPossibleScore);
	decreaseMaxTo(range.maxPossibleScore);
}
