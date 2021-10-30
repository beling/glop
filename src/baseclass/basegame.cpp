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

#include "basegame.h"

//constructor called by default by the derived class
BaseGame::BaseGame() {
	p_baseGame=0;
}

BaseGame::BaseGame(const Line &a) {
	initializePointer(Parameter::gameChoice);
	resetFrom(a);
}

BaseGame::BaseGame(const string &a) {
	initializePointer(Parameter::gameChoice);
	resetFrom(a);
}

BaseGame::BaseGame(int gameIndex) {
	initializePointer(gameIndex);
}

void BaseGame::initializePointer(int gameIndex) {
	//initialize the internal pointer with the correct game object
	if(gameIndex<(int)gameList().size()) { //security
		//create a new object of the correct game by cloning
		p_baseGame = (gameList()[gameIndex])->clone();
	} else {
		cout << "BaseGame error : unknown game" << endl;
		p_baseGame = 0;
	}
}

BaseGame::~BaseGame() {
	if(p_baseGame!=0) {
		delete p_baseGame;
		p_baseGame = 0;
	}
}

BaseGame::BaseGame(const BaseGame& a) {
	internalChildrenSet = a.internalChildrenSet;
	internalOptionsSet = a.internalOptionsSet;
	
	if(a.p_baseGame != 0) {
		p_baseGame = a.p_baseGame->clone();
	} else {
		p_baseGame = 0;
	}
}

BaseGame& BaseGame::operator=(const BaseGame& a) {
	if(this != &a) {
		internalChildrenSet = a.internalChildrenSet;
		internalOptionsSet = a.internalOptionsSet;
		
		//delete the old p_baseGame
		if(p_baseGame!=0) {
			delete p_baseGame;
			p_baseGame=0;
		}
		
		if(a.p_baseGame != 0) {
			p_baseGame = a.p_baseGame->clone();
		} else {
			p_baseGame = 0;
		}
	}
	
	return *this;
}

vector<BaseGame *>& BaseGame::gameList() {
	static vector<BaseGame *> internalList;
	return internalList;
}

//add a new game to the game list
//return the identifier for this game
int BaseGame::registerNewGame(BaseGame *a) {
	gameList().push_back(a);
	
	//return the index in the list
	return (gameList().size() - 1);
}

int BaseGame::gamesNumber() {
	return (int) (gameList().size());
}

//note:this function is never called (always reimplemented in the derived class)
BaseGame* BaseGame::clone() const {
	return new BaseGame(*this);
}

string BaseGame::exportToString() {
	if(p_baseGame!=0) {
		return p_baseGame->exportToString();
	} else {
		string voidString;
		return voidString;
	}
}

Line BaseGame::exportToLine() {
	if(p_baseGame!=0) {
		return p_baseGame->exportToLine();
	} else {
		Line voidLine;
		return voidLine;
	}
}

//optional string displayed in the interface
//return a void string by default
string BaseGame::exportOptionalString() {
	if(p_baseGame!=0) {
		return p_baseGame->exportOptionalString();
	} else {
		string voidString;
		return voidString;
	}
}

//!< title for the column displaying the optional string
string BaseGame::optionalStringTitle() {
	if(p_baseGame!=0) {
		return p_baseGame->optionalStringTitle();
	} else {
		return string("Cells "); //title by default
	}
}

void BaseGame::resetFrom(const Line &a) {
	if(p_baseGame!=0) {
		p_baseGame->resetFrom(a);
	}
}

void BaseGame::resetFrom(const string &a) {
	if(p_baseGame!=0) {
		p_baseGame->resetFrom(a);
	}
}

bool BaseGame::is_terminal_position() {
	if(p_baseGame!=0) {
		return p_baseGame->is_terminal_position();
	} else {
		//no special meaning
		return true;
	}
}

//return a heuristic value for the traversal initialization
//if the function is not implemented in a game, the default value is 1
double BaseGame::traversalHeuristic() {
	if(p_baseGame!=0) {
		return p_baseGame->traversalHeuristic();
	} else {
		return 10;		//default value
	}
}

win_loss BaseGame::Victory() {
	if(p_baseGame!=0) {
		return p_baseGame->Victory();
	} else {
		//no special meaning
		return false;
	}
}

//canonization
void BaseGame::canonize() {
	if(p_baseGame!=0) {
		return p_baseGame->canonize();
	}
}

//Category of the game.
//By default, return ImpartialGame
int BaseGame::gameCategory() {
	if(p_baseGame!=0) {
		return p_baseGame->gameCategory();
	} else {
		return GameType::ImpartialGame;
	}
}

//function to work around the problem of games having or not a terminal character in their representation
//TODO : the terminal character is useless and no game should use one
//convention : 0 means no terminal character and is the default value
int BaseGame::lastCharacter() {
	if(p_baseGame!=0) {
		return p_baseGame->lastCharacter();
	} else {
		return 0;
	}
}

//list of children for the position
set<Line>& BaseGame::childrenSet() {
	if(p_baseGame!=0) {
		return p_baseGame->internalChildrenSet;
	} else {
		return internalChildrenSet;
	}
}

void BaseGame::computeChildrenSet() {
	if(p_baseGame!=0) {
		p_baseGame->computeChildrenSet();
	} else {
		//default case if the function is not reimplemented
		//this is the case of a game that implements only computeOptionsSet
		computeOptionsSet();
		set<PositionString>::iterator Pi;
		for(Pi=optionsSet().begin(); Pi!=optionsSet().end(); Pi++) {
			BaseGame g(*Pi);
			childrenSet().insert(g.exportToLine());
		}
	}
}

//list of options for the position
set<PositionString>& BaseGame::optionsSet() {
	if(p_baseGame!=0) {
		return p_baseGame->internalOptionsSet;
	} else {
		return internalOptionsSet;
	}
}

void BaseGame::computeOptionsSet() {
	if(p_baseGame!=0) {
		p_baseGame->computeOptionsSet();
	} else {
		//default case if the function is not reimplemented
		//this is the case of a game that implements only computeChildrenSet
		computeChildrenSet();
		set<Line>::iterator Li;
		for(Li=childrenSet().begin(); Li!=childrenSet().end(); Li++) {
			BaseGame g(*Li);
			optionsSet().insert(g.exportToString());
		}
	}
}

void BaseGame::computeOrderParameters(MoveOrdering& result) {
	if(p_baseGame!=0) {
		return p_baseGame->computeOrderParameters(result);
	}
}

//by default, a game has no multicomponents
bool BaseGame::isMultiComponents() {
	if(p_baseGame!=0) {
		return p_baseGame->isMultiComponents();
	} else {
		return false;
	}
}

list<Line> BaseGame::sumComponents() {
	if(p_baseGame!=0) {
		return p_baseGame->sumComponents();
	} else {
		//by default, return only the position (probably useless)
		list<Line> result;
		result.push_back(exportToLine());
		return result;
	}
}

//(optional) compress the given string
string BaseGame::compress(const string &input) {
	if(p_baseGame!=0) {
		return p_baseGame->compress(input);
	} else {
		//by default, do nothing (return the same string)
		return input;
	}
}

//(optional) uncompress the given string
string BaseGame::uncompress(const string &input) {
	if(p_baseGame!=0) {
		return p_baseGame->uncompress(input);
	} else {
		//by default, do nothing (return the same string)
		return input;
	}
}

//(optional) draw a graphic representation of the game
void BaseGame::paint(DisplayPainter &painter) {
	if(p_baseGame!=0) {
		return p_baseGame->paint(painter);
	}
	//by default, do nothing
}

//(optional) indicate if graphics are available
//only games WITHOUT graphics must reimplement this function
bool BaseGame::isPaintable() {
	if(p_baseGame!=0) {
		return p_baseGame->isPaintable();
	} else {
		//by default, return true
		//it means that graphics are available
		return true;
	}
}

//by default, the available score is 0
//this function is used only by games with a notion of score
ScoreRange BaseGame::availableScore(bool maxScoreOnly) {
	if(p_baseGame!=0) {
		return p_baseGame->availableScore(maxScoreOnly);
	} else {
		ScoreRange s;
		return s;	//default value if the function is not reimplemented
	}
}

//(optional) indicate if the position should be stored permanently or not
//by default, a position needs to be stored permanently
bool BaseGame::isPermanentPosition() {
	if(p_baseGame!=0) {
		return p_baseGame->isPermanentPosition();
	} else {
		return true;
	}
}

Interface BaseGame::getParamDef() {
	if(p_baseGame!=0) {
		return p_baseGame->getParamDef();	
	}
	Interface voidList;
	return voidList;
}

void BaseGame::setParam() {
	if(p_baseGame!=0) {
		return p_baseGame->setParam();
	}
}

//return the name of the starting position, but also stores it in startPostionString
//it will be saved in xml files
string BaseGame::getPositionName() {
	if(p_baseGame!=0) {
		return p_baseGame->getPositionName();
	}
	string voidString;
	return voidString;
}

string gameString(const Line &a) {
	return BaseGame(a).exportToString();
}

string gameCompress(const string &input) {
	return BaseGame(Parameter::gameChoice).compress(input);
}

string gameUncompress(const string &input) {
	return BaseGame(Parameter::gameChoice).uncompress(input);
}