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

#ifndef BASEGAME_H
#define BASEGAME_H

#include "../gameglobal.h"
#include "interface.h"
#include "../moveordering.h"

class DisplayPainter;

//! Base class for all games
//! To implement a new game, create a derived class of BaseGame and reimplement the virtual functions
//! This class has been designed to be used as an object. It contains a pointer to the real object.
class BaseGame {
  private:
	BaseGame * p_baseGame;
	static vector<BaseGame *>& gameList();
	void initializePointer(int gameIndex);
	set<Line> internalChildrenSet;
	set<PositionString> internalOptionsSet;
	
  public:
	BaseGame();
	BaseGame(const Line &a);
	BaseGame(int gameIndex);
	BaseGame(const string &a);
	virtual ~BaseGame();
	
	//! @name Secured object copy
	//@{
	BaseGame(const BaseGame& a);				//!< copy constructor
	BaseGame& operator=(const BaseGame& a);	//!< assignment constructor
	virtual BaseGame* clone() const;			//!< clone function
	//@}
	
	static int registerNewGame(BaseGame *a);  //!< add a new game to the list of available games
	static int gamesNumber();
	
	//! @name String and Line representation
	//@{
	virtual string exportToString();			//!< string representation of the game
	virtual Line exportToLine();				//!< line representation of the game
	virtual string exportOptionalString();		//!< optional string displayed in the interface
	virtual string optionalStringTitle();		//!< title for the column displaying the optional string
	virtual void resetFrom(const Line &a);		//!< reset the object
	virtual void resetFrom(const string &a);	//!< reset the object
	//@}
	
	virtual int gameCategory(); //!< Category of the game (Impartial, PartialWL, PartialWLD...)
	
	//functions to treat the problem of the character at the end of a position
	//TODO : we should delete the terminal character of games having one
	//convention : 0 means no terminal character
	virtual int lastCharacter();
	
	//! @name Children
	//@{
	set<Line>& childrenSet();				//!< list of children for the position
	virtual void computeChildrenSet();		//!< this function needs to be reimplemented
	
	//use of string instead of Line (with a new type to facilitate later change)
	//in the end, these functions will replace childrenSet and computeChildrenSet
	set<PositionString>& optionsSet();
	virtual void computeOptionsSet();
	//@}
	
	virtual void canonize();  //!< canonization

	//! Parameters used to sort positions of the given game : it is used to set the  priorities during the computation
	virtual void computeOrderParameters(MoveOrdering& result);
	
	virtual bool isMultiComponents();		//!< indicate if there are multiple components
	virtual list<Line> sumComponents();		//!< independent components of the disjunctive sum

	virtual bool is_terminal_position();	//!< indicate if the game is finished (no moves possible)
	virtual double traversalHeuristic();	//! Heuristic for the traversal of the tree
	virtual win_loss Victory();  			//!< indicate which player has won
	
	virtual string compress(const string &input);		//!< (optional) compress the given string
	virtual string uncompress(const string &input);		//!< (optional) uncompress the given string
	virtual void paint(DisplayPainter &painter);		//!< (optional) draw a graphic representation of the game
	virtual bool isPaintable();							//!< (optional) indicate if graphics are available
	
	virtual bool isPermanentPosition();		//!< (optional) indicate if the position should be stored permanently or not
	
	//optional function for games with score
	virtual ScoreRange availableScore(bool maxScoreOnly);			//!< remaining available score of the position
	
	//! @name Parameter interface
	//@{
	virtual Interface getParamDef();	//!< Define the interface for the parameters of the game
	virtual void setParam();  			//!< Read the parameters value set by the user in the interface
	virtual string getPositionName();	//!< Name of the start position set by the user in the interface
	//@}
};

//usefull function to avoid complicated code syntax
string gameString(const Line &a);

//usefull function to avoid complicated code syntax
string gameCompress(const string &input);
string gameUncompress(const string &input);

//the following macro must be included in the public section of any class derived from BaseNode
//It provides the clone, access and register mecanism for the node
#define GAME_REGISTER_MACRO(GAME_NAME) \
	virtual BaseGame* clone() const { return new GAME_NAME(*this); }; \
	GAME_NAME(int) { registerNewGame(this); }; \
	static GAME_NAME root;
	
#endif
